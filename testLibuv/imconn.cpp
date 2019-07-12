/*
 * imconn.cpp
 *
 *  Created on: 2013-6-5
 *      Author: ziteng@mogujie.com
 */

#include "imconn.h"

static std::shared_ptr<CImConn> FindImConn(ConnMap_t* imconn_map, net_handle_t handle)
{
    std::shared_ptr<CImConn> pConn = NULL;
	ConnMap_t::iterator iter = imconn_map->find(handle);
	if (iter != imconn_map->end())
	{
		pConn = iter->second;
		//if(pConn){pConn->AddRef();}
	}

	return pConn;
}

static std::shared_ptr<CImConn> FindImConn(ConnMap_t* imconn_map, uv_stream_t* handle)
{
    std::shared_ptr<CImConn> pConn = NULL;
	ConnMap_t::iterator iter = imconn_map->find(handle);
	if (iter != imconn_map->end())
	{
		pConn = iter->second;
		//if(pConn){pConn->AddRef();}
	}

	return pConn;
}

void imconn_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	NOTUSED_ARG(handle);
	NOTUSED_ARG(pParam);
;
	if (!callback_data)
		return;

	ConnMap_t* conn_map = (ConnMap_t*)callback_data;
	std::shared_ptr<CImConn> pConn = FindImConn(conn_map, handle);
	if (!pConn)
		return;

	//log("msg=%d, handle=%d ", msg, handle);

	switch (msg)
	{
	case NETLIB_MSG_CONFIRM:
		pConn->OnConfirm();
		break;
	case NETLIB_MSG_READ:
		pConn->OnRead();
		break;
	case NETLIB_MSG_WRITE:
		pConn->OnWrite();
		break;
	case NETLIB_MSG_CLOSE:
		pConn->OnClose();
		break;
	default:
		log("!!!imconn_callback error msg: %d ", msg);
		break;
	}

	//pConn->ReleaseRef();
}

//////////////////////////
CImConn::CImConn()
{
	//log("CImConn::CImConn ");

	m_busy = false;
	m_handle = NETLIB_INVALID_HANDLE;
	m_recv_bytes = 0;

	m_last_send_tick = m_last_recv_tick = get_tick_count();
}

CImConn::~CImConn()
{
	//log("CImConn::~CImConn, handle=%d ", m_handle);
}

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

int CImConn::WriteData(void* data, int len)
{
	m_last_send_tick = get_tick_count();

    write_req_t *req = (write_req_t*)malloc(sizeof(write_req_t));
    req->buf = uv_buf_init((char*)malloc(len), len);
    memcpy(req->buf.base, data, len);

    std::cout << "ready data = " << std::string(req->buf.base, req->buf.len).c_str() << std::end;
    uv_write((uv_write_t*) req, (uv_stream_t*)m_stream, &req->buf, 1, [](uv_write_t* req, int status){   
             if(status >= 0) {                                                                      
                std::cout << "write successfully" <<endl;                                          
             }else {                                                                                
                 std::cout << "write faile"<< endl;                                                 
             }

             if(req) {                                                                              
                 free(((write_req_t*)req)->buf.base);                                               
                 free(req);                                                                         
                 req = NULL;                                                                
             }
    });
    return len;//或者返回0,因为这个写操作真实状态由uv_write回调返回
}

int CImConn::Send(void* data, int len)
{
	m_last_send_tick = get_tick_count();
//	++g_send_pkt_cnt;

	if (m_busy)
	{
		m_out_buf.Write(data, len);
		return len;
	}

	int offset = 0;
	int remain = len;
	while (remain > 0) {
		int send_size = remain;
		if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) {
			send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
		}

		int ret = netlib_send(m_handle, (char*)data + offset , send_size);
		if (ret <= 0) {
			ret = 0;
			break;
		}

		offset += ret;
		remain -= ret;
	}

	if (remain > 0)
	{
		m_out_buf.Write((char*)data + offset, remain);
		m_busy = true;
		log("send busy, remain=%d ", m_out_buf.GetWriteOffset());
	}
    else
    {
        OnWriteCompelete();
    }

	return len;
}

void CImConn::OnTimer(uint64_t curr_tick)
{
	if(!curr_tick)
		return;
}

void CImConn::OnRead(char* buf, ssize_t len)
{
    //数据已经由libuv读出来了，只要在这里把它存储在应用层接收缓冲就行;存下来之前要判断缓冲剩余长度
    uint32_t free_buf_len = m_in_buf.GetAllocSize() - m_in_buf.GetWriteOffset();
    if (free_buf_len < len)
    {
        m_in_buf.Extend(len);
    }
    //把数据塞到m_in_buf
    memcpy(m_in_buf.GetBuffer() + m_in_buf.GetWriteOffset(), buf , len);
    m_in_buf.IncWriteOffset(len);
    
    m_recv_bytes += len;
    m_last_recv_tick = get_tick_count();

    std::shared_ptr<CImPdu> pPdu = NULL;
    try
    {
		while ( ( pPdu = CImPdu::ReadPdu(m_in_buf.GetBuffer(), m_in_buf.GetWriteOffset()) ) )
		{
                uint32_t pdu_len = pPdu->GetLength();
                pPdu->SetSockStream(m_stream);

                HandlePdu(pPdu);
                
                m_in_buf.Read(NULL, pdu_len);
		}
	} catch (CPduException& ex) {
		ErrLog("!!!catch exception, sid=%u, cid=%u, err_code=%u, err_msg=%s, close the connection ",
				ex.GetServiceId(), ex.GetCommandId(), ex.GetErrorCode(), ex.GetErrorMsg());
        if (pPdu) {
            pPdu.reset();
        }
        OnClose();
	}

}

void CImConn::OnRead()
{
	for (;;)
	{
		uint32_t free_buf_len = m_in_buf.GetAllocSize() - m_in_buf.GetWriteOffset();
		if (free_buf_len < READ_BUF_SIZE)
			m_in_buf.Extend(READ_BUF_SIZE);

		int ret = netlib_recv(m_handle, m_in_buf.GetBuffer() + m_in_buf.GetWriteOffset(), READ_BUF_SIZE);
		
		if (ret <= 0)
			break;

		m_recv_bytes += ret;
		m_in_buf.IncWriteOffset(ret);

		m_last_recv_tick = get_tick_count();
	}

    std::shared_ptr<CImPdu> pPdu = NULL;
    try
    {
		while ( ( pPdu = CImPdu::ReadPdu(m_in_buf.GetBuffer(), m_in_buf.GetWriteOffset()) ) )
		{
                uint32_t pdu_len = pPdu->GetLength();
                pPdu->SetSockHandle(m_handle);

                HandlePdu(pPdu);

                m_in_buf.Read(NULL, pdu_len);
                //pPdu.reset();
		}
	} catch (CPduException& ex) {
		ErrLog("!!!catch exception, sid=%u, cid=%u, err_code=%u, err_msg=%s, close the connection ",
				ex.GetServiceId(), ex.GetCommandId(), ex.GetErrorCode(), ex.GetErrorMsg());
        if (pPdu) {
            pPdu.reset();
        }
        OnClose();
	}
}

void CImConn::OnWrite()
{
	if (!m_busy)
		return;

	while (m_out_buf.GetWriteOffset() > 0) {
		int send_size = m_out_buf.GetWriteOffset();
		if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) {
			send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
		}

		int ret = netlib_send(m_handle, m_out_buf.GetBuffer(), send_size);
		if (ret <= 0) {
			ret = 0;
			break;
		}

		m_out_buf.Read(NULL, ret);
	}

	if (m_out_buf.GetWriteOffset() == 0) {
		m_busy = false;
	}

	log("onWrite, remain=%d ", m_out_buf.GetWriteOffset());
}

void CImConn::HandlePdu(CImPdu* pPdu) 
{
 	if(!pPdu)
		return;
}

void CImConn::HandlePdu(std::shared_ptr<CImPdu> pPdu) 
{
	if(!pPdu)
		return;
}





