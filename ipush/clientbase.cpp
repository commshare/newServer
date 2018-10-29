#include "clientbase.h"
#include "ssl_event.h"
#include "postDataSender.h"
#include "CApnsPostData.h"

CClientBase::CClientBase()
	:m_clientType(PUSH_CLIENT_TYPE_UNKNOWN), m_pPostMgr(nullptr)
{
	m_pSslEventDispatch = GetSslEventDispatch();	//默认值
}

CClientBase::~CClientBase()
{
	if (m_pPostMgr)
	{
		delete m_pPostMgr;
	}
}

std::shared_ptr<CSslEventDispatch> CClientBase::GetSslEventDispatch()
{
	static std::shared_ptr<CSslEventDispatch> pSslEventDispatch = std::shared_ptr<CSslEventDispatch>(new CSslEventDispatch);
	return pSslEventDispatch;
}

bool CClientBase::AddTask(const im::PSvrMsg& msg)
{
	bool bRet = false;
	if (m_postSenders.empty()) return bRet;

	shared_ptr<CApnsPostData> pPostData = GeneratePostDataFromMsg(msg);
	if (!pPostData)
	{
		WarnLog("###GenerateAPNsPostData false");
		return false;
	}

	return AddTask(pPostData);
}

bool CClientBase::AddTask(shared_ptr<CApnsPostData> data)
{
	bool bRet = false;
	if (m_postSenders.empty()) return bRet;

	int index = BKDRHash(data->sToId.c_str()) % m_postSenders.size();
	return m_postSenders[index]->PostData(data);

}

bool CClientBase::startEpoll()
{
	if (!m_pSslEventDispatch)
	{
		ErrLog("m_pSslEventDispatch is nullptr");

		return false;
	}
	m_pSslEventDispatch->StartDispatch();

	return true;
}

bool CClientBase::Start()
{
	if (!m_pPostMgr)
	{
		ErrLog("m_pPostMgr is nullptr");
		return false;
	}

	if (!m_pSslEventDispatch)
	{
		ErrLog("m_pSslEventDispatch is nullptr");
		return false;
	}

	//pthread_t tid;
	//int i;
	//创建epool线程侦听socket收发
	startEpoll();



	//创建推送消息的发送线程池

	for (unsigned int i = 0; i < m_postSenders.size(); ++i)
	{
		m_postSenders[i]->StartThread();
	}

	//创建Socket连接,用来推送
	m_pPostMgr->Start();
	return true;
}
