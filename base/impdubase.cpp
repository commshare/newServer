/*****************************************************************************************
Filename:  impdubase.h
Author: jack			Version: im-1.0 		Date:2017/6/26
Description:   网络层PDU数据类实现，负责基于protobuf协议包头、包体的数据封装、数据包的读写
			  访问等。
*****************************************************************************************/
#include "util.h"
#include "impdubase.h"

CImPdu::CImPdu()
{
    m_pdu_header.nPID = 'US';
    m_pdu_header.nVersion = IM_PDU_VERSION;
    m_pdu_header.bEncrypt = 0;
    m_pdu_header.nCmdId = 0;
    m_pdu_header.nBodySize = 0;
	memset(&m_pdu_header.sessionId,0,SIZE_PDU_HDR_SESSIONID);
}

uchar_t* CImPdu::GetBuffer()
{
    return m_buf.GetBuffer();
}

uint16_t CImPdu::GetLength()
{
    return m_buf.GetWriteOffset();
}

uchar_t* CImPdu::GetBodyData()
{
    return m_buf.GetBuffer() + IM_PDU_HEADER_LEN;
}

uint16_t CImPdu::GetBodyLength()
{
    uint16_t body_length = 0;
    body_length = m_buf.GetWriteOffset() - IM_PDU_HEADER_LEN;
    return body_length;
}

void CImPdu::WriteHeader()   			//Write the protobuf header , fix length 
{
	uchar_t* buf = GetBuffer();

    CByteStream::WriteUint16(buf + OFFSET_PDU_HDR_PID, m_pdu_header.nPID);
    CByteStream::WriteUint8(buf + OFFSET_PDU_HDR_VERSION, m_pdu_header.nVersion);
	CByteStream::WriteSession(buf + OFFSET_PDU_HDR_SESSIONID, 
				m_pdu_header.sessionId.Uid_Item.code,SIZE_PDU_HDR_SESSIONID);
    CByteStream::WriteUint8(buf + OFFSET_PDU_HDR_ENCRYPT, m_pdu_header.bEncrypt);    
    CByteStream::WriteUint16(buf + OFFSET_PDU_HDR_CMDID, htons(m_pdu_header.nCmdId));
    CByteStream::WriteUint16(buf + OFFSET_PDU_HDR_BODYSIZE, htons(GetBodyLength()));
	
}
void CImPdu::SetVersion(uint8_t version)
{
	uchar_t* buf = GetBuffer();
	CByteStream::WriteUint8(buf + OFFSET_PDU_HDR_VERSION, version);
	m_pdu_header.nVersion = version;
}

void CImPdu::SetPID(uint16_t flag)
{
	uchar_t* buf = GetBuffer();
	CByteStream::WriteUint16(buf + OFFSET_PDU_HDR_PID, flag);
	m_pdu_header.nPID = flag;
}

void CImPdu::SetEncrypt(uint8_t bEncrypt)
{
	uchar_t* buf = GetBuffer();
	CByteStream::WriteUint8(buf + OFFSET_PDU_HDR_ENCRYPT, bEncrypt);
	m_pdu_header.bEncrypt = bEncrypt;
}

void CImPdu::SetCommandId(uint16_t command_id)
{
    uchar_t* buf = GetBuffer();
    CByteStream::WriteUint16(buf + OFFSET_PDU_HDR_CMDID, htons(command_id));
	m_pdu_header.nCmdId = command_id;
}

void CImPdu::SetSessionId(UidCode_t sessionId)
{
    uchar_t* buf = GetBuffer();
	CByteStream::WriteSession(buf + OFFSET_PDU_HDR_SESSIONID, 
				sessionId.Uid_Item.code,SIZE_PDU_HDR_SESSIONID);  
	m_pdu_header.sessionId = sessionId;
}

int CImPdu::ReadPduHeader(uchar_t* buf, uint32_t len)
{
	int ret = -1;
	if (len >= IM_PDU_HEADER_LEN && buf) {
		CByteStream is(buf, len);

		is >> m_pdu_header.nPID;
		is >> m_pdu_header.nVersion;
		is.ReadSession(m_pdu_header.sessionId.Uid_Item.code,SIZE_PDU_HDR_SESSIONID); 
		is >> m_pdu_header.bEncrypt;
		is >> m_pdu_header.nCmdId;
        is >> m_pdu_header.nBodySize;

		m_pdu_header.nCmdId = ntohs(m_pdu_header.nCmdId);
		m_pdu_header.nBodySize= ntohs(m_pdu_header.nBodySize);

		//DbgLog("===Pdu cmd : 0x%x，body size:%d ==",m_pdu_header.nCmdId,m_pdu_header.nBodySize);

		ret = 0;
	}

	return ret;
}

std::shared_ptr<CImPdu> CImPdu::ReadPdu(uchar_t *buf, uint32_t len)
{
    uint32_t pdu_len = 0;
    if (len < 1) return NULL; 
    if (!IsPduAvailable(buf, len, pdu_len))   //Check the pdu whether or not valid. 
        return NULL;

    std::shared_ptr<CImPdu> pPdu = std::make_shared<CImPdu> ();

    pPdu->Write(buf, pdu_len);     //Write protobuf with it's head to buf.
    pPdu->ReadPduHeader(buf, IM_PDU_HEADER_LEN);

    return pPdu;
}


bool CImPdu::IsPduAvailable(uchar_t* buf, uint32_t len, uint32_t& pdu_len)
{
	if (len < IM_PDU_HEADER_LEN)
		return false;

	uint16_t nBodyLen = CByteStream::ReadUint16(buf+OFFSET_PDU_HDR_BODYSIZE);
	pdu_len = ntohs(nBodyLen) + IM_PDU_HEADER_LEN;
	//DbgLog("Pdu available len %d,%d",len,pdu_len);
	if (pdu_len > len) // it is error if pdu len >  total received len. 
	{
		return false;
	}
    
    if(0 == pdu_len)   //  it is exception if pdu len == 0
    {
        throw CPduException(1, "pdu_len is 0");
    }

	return true;
}

void CImPdu::SetPBMsg(const google::protobuf::MessageLite* msg)
{
    m_buf.Read(NULL, m_buf.GetWriteOffset()); //Reset body and allocate memory space
    m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    uint32_t msg_size = msg->ByteSize();
    uchar_t* szData = new uchar_t[msg_size];

    if (!msg->SerializeToArray(szData, msg_size)) //Serialize msg structure. 
    {
        log("pb msg miss required fields.");
    }
    
    m_buf.Write(szData, msg_size);
    delete []szData;
    WriteHeader();
}

void CImPdu::SetPBWithoutMsg(uint16_t nCmdId,UidCode_t sessionId)
{
    m_buf.Read(NULL, m_buf.GetWriteOffset()); //Reset body and allocate memory space
    m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	WriteHeader();
	
	SetCommandId(nCmdId);
	SetSessionId(sessionId);
}


