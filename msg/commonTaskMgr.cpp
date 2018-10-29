/******************************************************************************
Filename: commonTaskMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/10/08
Description: 
******************************************************************************/
#include "commonTaskMgr.h"
#include "configfilereader.h"
#include "threadpool.h"
#include "util.h"

CCommonTask::CCommonTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, commobTaskCallBack callBack, void* para)
	: m_pTaskData(pGrpTask), m_callBack(callBack), m_pCallBackParas(para)
{

}

void CCommonTask::run()
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();
	if (m_callBack)
	{
		m_callBack(m_pTaskData, m_pCallBackParas);
	}
	//DbgLog("thread %lu run CCommonTask %p, use %llu usecond\r\n", pthread_self(), this, elspsedTimer.elapsed());
}


/////////////////////////////////////////////////////////////////////

CCommonTaskMgr::~CCommonTaskMgr()
{

}

int CCommonTaskMgr::Init(CConfigFileReader* pReader)
{
	int common_thread_num = 40;
	if (pReader != NULL)
	{
		CConfigFileReader &config_file = *pReader;
		char* str_common_thread_num = config_file.GetConfigName("common_thread_num");
		common_thread_num = str_common_thread_num ? atoi(str_common_thread_num) : common_thread_num;
	}

	m_pCommonThreadPool = new CImThreadPool();
	//��������ɹ�����ʼ��ʧ�ܣ��򷵻�NULL
	if (m_pCommonThreadPool != NULL && m_pCommonThreadPool->Init(common_thread_num) != 0)
	{
		ErrLog("commonthread created failed, size = %d", common_thread_num);
		delete m_pCommonThreadPool;
		m_pCommonThreadPool = NULL;
		return 1;
	}
	log("CCommonTaskMgr created %d threads", common_thread_num);

	m_bHasInit = true;
	return 0;
}

CImThreadPool* CCommonTaskMgr::GetCommonThreadPoolInstance()
{
	return m_pCommonThreadPool;
}

bool CCommonTaskMgr::InsertCommonTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, commobTaskCallBack callBack, void* para, int index)
{
	//����task,��task��ִ�в��붯������ɺ�ص�
	CCommonTask* pInsertTask = new CCommonTask(pGrpTask, callBack, para);
	
	CImThreadPool* pThreadPool = GetCommonThreadPoolInstance();
	if (NULL == pInsertTask || NULL == pThreadPool)	//�������ʧ�ܣ����ûص�����
	{
		if (callBack)
		{
			callBack(pGrpTask, para);
		}
		return false;
	}
	pThreadPool->AddTask(pInsertTask, index);
	return true;
}


//bool CCommonTaskMgr::InsertMsgHandleTask(const std::shared_ptr<CImPdu>& pPdu, msgHandleTaskCallBack callBack, void* para)
//{
//	//����task,��task��ִ�в��붯������ɺ�ص�
//	CMsgHandleTask* pInsertTask = new CMsgHandleTask(pPdu, callBack, para);
//
//	CImThreadPool* pThreadPool = GetCommonThreadPoolInstance();
//	if (NULL == pInsertTask || NULL == pThreadPool)	//�������ʧ�ܣ����ûص�����
//	{
//		if (callBack)
//		{
//			callBack(pPdu, para);
//		}
//		return false;
//	}
//	pThreadPool->AddTask(pInsertTask);
//	return true;
//}


void CCommonTaskMgr::Stop()
{
	if (m_pCommonThreadPool)
	{
		m_pCommonThreadPool->StopThreads();
		m_pCommonThreadPool->waitForDone();		//���̵߳ȴ����е�mongoThread����
	}
}

CCommonTaskMgr::CCommonTaskMgr()
{

}

CImThreadPool* CCommonTaskMgr::m_pCommonThreadPool = NULL;

bool CCommonTaskMgr::m_bHasInit = false;


////////////////////////////////////////////////////////////////////
//void CMsgHandleTask::run()
//{
//	CUsecElspsedTimer elspsedTimer;
//	elspsedTimer.start();
//	if (m_callBack)
//	{
//		m_callBack(m_pTaskData, m_pCallBackParas);
//	}
//	DbgLog("thread %lu run CMsgHandleTask %p, use %llu usecond\r\n", pthread_self(), this, elspsedTimer.elapsed());
//}
//
//CMsgHandleTask::CMsgHandleTask(const std::shared_ptr<CImPdu>& pPdu, msgHandleTaskCallBack callBack, void* para)
//	: m_pTaskData(pPdu), m_callBack(callBack), m_pCallBackParas(para)
//{
//
//}
