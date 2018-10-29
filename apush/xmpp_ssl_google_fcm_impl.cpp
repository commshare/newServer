/**
 * file xmpp_ssl_google_fcm_impl.cpp 
 *  create by liulang 2017-08-26
 */
#include "xmpp_ssl_google_fcm_impl.h"
#include "xmpp_google_fcm_protoc.h"
#include "xmpp_recv_xml_json_parse.h"

#ifndef PRODUCT
static const string xmpp_fcm_webSite = "fcm-xmpp.googleapis.com";
static short xmpp_fcm_test_port = 5236;

//发送Sender ID
const string xmpp_handshake_sendId = "846530884595";
//发送秘钥Legacy server key 
const string xmpp_handshake_pwd = "AIzaSyBj1FIhdGqDh-1zjFhGARs2I2bP2AlL6ik";
#else
static const string xmpp_fcm_webSite = "fcm-xmpp.googleapis.com";
static short xmpp_fcm_test_port = 5235;
//发送Sender ID
const string xmpp_handshake_sendId = "846530884595";
//发送秘钥Legacy server key 
const string xmpp_handshake_pwd = "AIzaSyBj1FIhdGqDh-1zjFhGARs2I2bP2AlL6ik";
#endif

//epoll中,socket回调
void CXmppImpl::SockEventCallBack(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	NOTUSED_ARG(pParam);
	if(!callback_data)
	{
		ErrLog("callback_data");
		return;
	}
	CXmppImpl *xmpp = (CXmppImpl *)callback_data;

	//可读,数据到来
	if (msg == NETLIB_MSG_READ)
	{
		xmpp->OnRead(handle);
	}
	//可写,对端缓冲区,有空间可读
	else if(msg == NETLIB_MSG_WRITE)
	{
		xmpp->OnWrite();
		return;
	}
	//对端关闭连接
	else if(msg == NETLIB_MSG_CLOSE)
	{
		xmpp->OnClose();
	}
	//未知状态
	else
	{

	}
}

static int xmppNotify = 0;
int CXmppImpl::OnFcmNotify(string &completeStr)
{
	xmpp_recv_xml_json_parse xmljosn(completeStr.c_str(), completeStr.size());

	int index = 0;
	string msgId;

	bool bRet = xmljosn.GetIndexAndMsgId(index, msgId);
	InfoLog("recv:%s, index:%d, msgid:%s", bRet?"ack":"nack", index, msgId.c_str());

	shared_ptr<APushData> sharedApushData = m_sendDataMapMgr.Delete(index);
	if (!sharedApushData)
	{
		ErrLog("m_sendDataMapMgr.Delete(index) return null");
		return -1;
	}

	PaPushData data = sharedApushData.get();
	if (!data)
	{
		ErrLog("m_sendDataMapMgr.Delete(index) return null");
		return -1;
	}
	if (data->msgId != msgId)
	{
		ErrLog("data->msgId:%s != msgId:%s", data->msgId.c_str(), msgId.c_str());
	}

	if (bRet)
	{
		data->retStatus = EXCEPT_ERR;
	}
	else
	{
		data->retStatus = NON_ERR;
	}

	DbgLog("CXmppImpl::OnFcmNotify:%d", ++xmppNotify);
	m_onNotifyCallBack(m_userData, sharedApushData);

	return 0;
}


//note: not Thread safe
int CXmppImpl::OnRead(uint32_t)
{
	while (1)
	{
		memset(m_recvBuf, 0, XMPP_RECVBUF_SIZE);

		if(m_sslSock.Recv(m_recvBuf, XMPP_RECVBUF_SIZE) <= 0)
		{
			WarnLog("m_sslSock Recv");
			return -1;
		}

		string strTmp = m_recvBuf;

		//粘包处理, 只处理正确情况,其他情况一律丢包处理
		while (strTmp.size() > 0)
		{
			if (m_packageCache.empty())
			{
				int beginPos = strTmp.find(xmpp_msg_xml_head);
				int endPos = strTmp.find(xmpp_msg_xml_tail);


				//心跳包,或是半包
				if (endPos < 0 && beginPos < 0)
				{
					m_packageCache = strTmp;

					//心跳包
					if (m_packageCache == string(" "))
					{
						InfoLog("xmpp heartbeat rate:%u!", time(NULL) - m_heartBeatTime);
						m_heartBeatTime = time(NULL);
						m_packageCache.clear();
					}
					
					break;
				}

				//正常包
				if (beginPos>=0 && endPos > 0 && endPos > beginPos)
				{
					string completeStr = strTmp.substr(beginPos, endPos + xmpp_msg_xml_tail.size());

					strTmp = strTmp.substr(endPos + xmpp_msg_xml_tail.size());

					OnFcmNotify(completeStr);
					
					continue;
				}

				//前面废包,后面半包
				if (beginPos >= 0 && endPos < 0)
				{
					m_packageCache = strTmp.substr(beginPos);
					strTmp.clear();
					ErrLog("unknow Recv strTmp:%s \nm_recvBuf:%s", strTmp.c_str(), m_recvBuf);
				}
				else //错误包
				{
					WarnLog("unknow Recv strTmp:%s \nm_recvBuf:%s", strTmp.c_str(), m_recvBuf);
				}
				//exit(-1);
				break;
			}
			//上次的残余包
			else
			{
				strTmp = m_packageCache + strTmp;
				m_packageCache.clear();
				/*
				int endPos = strTmp.find(xmpp_msg_xml_tail);
				if (endPos >= 0)
				{
					m_packageCache += strTmp.substr(0, endPos + xmpp_msg_xml_tail.size());

					strTmp = strTmp.substr(endPos + xmpp_msg_xml_tail.size());

					OnFcmNotify(m_packageCache);

					m_packageCache.clear();

					continue;
				}

				int beginPos = strTmp.find(xmpp_msg_xml_head);
				
				if (beginPos < 0)
				{
					m_packageCache += strTmp;
					strTmp.clear();
					continue;
				}
				else
				{
					m_packageCache.clear();
					WarnLog("beginPos:%d, end:%d, Recv m_recvBuf:%s,\n strTmp:%s,\n m_packageCache:%s", 
							beginPos, endPos, m_recvBuf, strTmp.c_str(), m_packageCache.c_str());
					break;
				}
				*/
			}
			
		}//while
		
		DbgLog("m_PackStreamQueue:%d, m_sendDataMapMgr:%d", m_PackStreamQueue.GetSize(), m_sendDataMapMgr.GetSize());
		//InfoLog("for test m_recvBuf %s", m_recvBuf);
	}
}


//CLock	m_LineWriteLock();	//操作socket时,为保证ssl安全,写操作,须串行执行
int CXmppImpl::_Write(const char *buf, int len)
{
	//CAutoLock lock(&m_LineWriteLock);
	if (!buf || len <= 0)
	{
		ErrLog("_Write");
		return -1;
	}

	if (!m_bHandhake)
	{
		WarnLog("not m_bHandhake");
		return -1;
	}
	//InfoLog("send:%s", buf);
	int iRet = m_sslSock.Send((void *)buf, len);
	//发送成功直接返回
	if(iRet == len)	//大部分情况
	{
		return iRet;
	}

	if (iRet == 0) //少部分
	{
		InfoLog("send block!");
	}

	if ( iRet > 0 && iRet < len) //暂时未发现,发现将是严重错误
	{
		ErrLog("send packet not compelte!");
	}

	if (iRet > 0)
	{
		InfoLog("send len:%d, str len:%d", iRet, len);
	}

	if (iRet < 0)//严重错业
	{
		ErrLog("_Write Send err need close!");
		//exit(-1);
	}

	return -1;
}


int CXmppImpl::OnWrite()
{

	return -1;
	//OnRead(0);

	while (1)
	{
		if (m_PackStreamQueue.GetSize() == 0)
		{
			return 0;
		}

		shared_ptr<APushData> ptr_data = m_PackStreamQueue.PopFront();

		if (!ptr_data)
		{
			ErrLog("m_PackStreamQueue PopFront nullptr");
		}
		APushData *data = ptr_data.get();

		if (!data)
		{
			ErrLog("m_PackStreamQueue data nullptr");
			return 0;
		}

		//handle queue cache 不必了.
		int index = m_sendDataMapMgr.Insert(ptr_data, data->mapIndex);
		if(index < -1)
		{
			WarnLog(" m_sendDataMapMgr.Insert(data);");
		}

		data->timeSend = time(nullptr);
		//InfoLog("send:%s", data->sendBuf.c_str());

		int iRet = _Write(data->sendBuf.c_str(), data->sendBuf.size());
		 //发送成功直接返回
		if(iRet > 0)
		{
			InfoLog("Send success, msgid:%s, toid:%s", data->msgId.c_str(), data->toId.c_str());
			//return iRet;
		}
		else
		{
			WarnLog("sock.Send, msgid:%s, toid:%s", data->msgId.c_str(), data->toId.c_str());
			m_PackStreamQueue.PushBack(ptr_data);
			break;
		}
	}

	return -1;
}

int CXmppImpl::OnClose()
{
	CloseXmpp();
	//m_sslSock.Close();
	//m_sslSock.ReConnect();

	//todo
	//检查cache中是否有数据fcm未给应答,或其原因造成的残存数据
	//m_sendDataMapMgr.CheckCache();
	return 0;
}


void CXmppImpl::CloseXmpp()
{

	InfoLog("CloseXmpp");
	m_bHandhake = false;
	m_sendDataMapMgr.Clear();
	//m_packageCache.clear();
	m_sslSock.Close();

	m_heartBeatTime = 0;
}

int CXmppImpl::Init(const char *pSendId, const char *pServerKey, OnNotifyCallBack_t func, void *userData)
{
	NOTUSED_ARG(pSendId);
	if(!pServerKey || !pServerKey)
	{
		ErrLog("!pServerKey || !pServerKey");
		return -1;
	}

	if (!func || !userData)
	{
		ErrLog("m_onNotifyCallBack || !m_userData");
		return -1;
	}
	
	m_onNotifyCallBack = func;
	m_userData = userData;

	if(!m_sslSock.Init())
	{
		ErrLog("m_sslSock->Init()");
		return -1;
	}

	m_bHandhake = false;
	m_heartBeatTime = 0;

	return 0;
}

int CXmppImpl::XmppHandshake()
{
	char recvBuf[1024];
	m_recvBuf[XMPP_RECVBUF_SIZE] = '\0';

	//m_sendDataMapMgr.Clear();

	if(m_sslSock.SslConnectWebSite(SockEventCallBack, 
		this, xmpp_fcm_webSite.c_str(), xmpp_fcm_test_port) <= 0) 
	{

		ErrLog("SslConnectWebSite");
		return -1;
	}

	//1. 客户端初始化流给服务器
	if(m_sslSock.Send((void *)xmpp_handshake_init.c_str(), xmpp_handshake_init.size()) < 0)
	{
		ErrLog("Send xmpp_handshake_init");
		return -1;
	}

	memset(recvBuf, 0, 1024);
	if(m_sslSock.Recv(recvBuf, 1024) < 0)
	{
		ErrLog("Recv xmpp_handshake_init");
		return -1;
	}
	InfoLog("Recv xmpp_handshake_init:%s", recvBuf);

	//2. 服务器通知客户端可用的验证机制
	char send[1024] = {0};
	memset(send, 0 , 1024);

	string loginstr;
	loginstr += '\0';
	loginstr += xmpp_handshake_sendId;
	loginstr += '\0';
	loginstr += xmpp_handshake_pwd;
	string key = base64_encode(loginstr);
	sprintf(send, xmpp_handshake_mechanism.c_str(), key.c_str());
	key = send;
	if(m_sslSock.Send((void *)key.c_str(), key.size()) < 0)
	{
		ErrLog("Send xmpp_handshake_mechanism");
		return -1;
	}

	memset(recvBuf, 0, 1024);
	if(m_sslSock.Recv(recvBuf, 1024) < 0)
	{
		ErrLog("Recv xmpp_handshake_mechanism");
		return -1;
	}
	InfoLog("Recv xmpp_handshake_mechanism:%s", recvBuf);

	//3. 客户端发起一个新的流给服务器
	if(m_sslSock.Send((void*)xmpp_handshake_stream.c_str(), xmpp_handshake_stream.size()) < 0)
	{
		ErrLog("Send xmpp_handshake_stream");
		return -1;
	}

	memset(recvBuf, 0, 1024);
	if(m_sslSock.Recv(recvBuf, 1024) < 0)
	{
		ErrLog("Recv xmpp_handshake_stream");
		return -1;
	}
	InfoLog("Recv xmpp_handshake_stream:%s", recvBuf);

	//4. 发送一个流头信息,并附上任何可用的特性(iq id)
	memset(send, 0 , 1024);
	static int iq_num = 1000;
	sprintf(send, xmpp_handshake_iq_num.c_str(), iq_num++);
	if(m_sslSock.Send((void *)send, strlen(send)) < 0)
	{
		ErrLog("Send xmpp_handshake_iq_num");
		return -1;
	}

	memset(recvBuf, 0, 1024);
	if(m_sslSock.Recv(recvBuf, 1024) < 0)
	{
		ErrLog("Recv xmpp_handshake_iq_num");
		return -1;
	}
	InfoLog("Recv xmpp_handshake_iq_num:%s", recvBuf);


	if (-1 == SetBlockOrNot(m_sslSock.GetSocket(), 1))
	{
		ErrLog("set blocking socket  error!");
		return -1;
	}

	//5. add to epoll ctl
	CSslEventDispatch::Instance()->AddEvent(&m_sslSock, 5*60*1000);

	m_bHandhake = true;
	m_heartBeatTime = time(NULL);
	return m_sslSock.GetSocket();
}

int CXmppImpl::CheckSession()
{
	return 0;
}

int CXmppImpl::CheckSendMapDataTime()
{

	return 0;
}



int CXmppImpl::AddTask(shared_ptr<APushData> shared_APushData)
{

	if (!shared_APushData)
	{
		ErrLog("shared_APushData is null");
		return -1;
	}
	
	APushData *data = shared_APushData.get();

	if (!data)
	{
		ErrLog("shared_APushData.get() is nu;;");
		return -1;
	}
	//handle queue cache
	int index = m_sendDataMapMgr.Insert(shared_APushData, data->mapIndex);
	if(index < -1)
	{
		m_sendDataMapMgr.Delete(data->mapIndex);
		ErrLog(" m_sendDataMapMgr.Insert(data)");
		return -1;
	}

	if(data->mapIndex != index)
	{
		ErrLog("m_sendDataMapMgr.Insert :%s", data->msgId.c_str());
		return -1;
	}

	data->timeSend = time(nullptr);

	//InfoLog("send:%s", data->sendBuf.c_str());	
	int iRet = _Write(data->sendBuf.c_str(), data->sendBuf.size());
	//发送成功直接返回
	if(iRet > 0)
	{
		InfoLog("Send success, msgid:%s, toid:%s", data->msgId.c_str(), data->toId.c_str());
		return iRet;
	}
	else
	{
		//InfoLog("sock.Send, msgid:%s, toid:%s", data->msgId.c_str(), data->toId.c_str());
		//m_PackStreamQueue.PushBack(shared_APushData);
	}
//	
//errRet:
	return iRet;
}


int CXmppImpl::_HandleXmppOnceAck(char *buf)
{
	NOTUSED_ARG(buf);
	return 0;
}

