/******************************************************************************
Filename: commonTaskMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/10/08
Description: 
******************************************************************************/
#include "offlineMsgTaskMgr.h"
#include "configfilereader.h"
#include "threadpool.h"
#include "util.h"

COfflineMsgTask::COfflineMsgTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, offlineMsgTaskCallBack callBack, void* para)
	: m_pTaskData(pGrpTask), m_callBack(callBack), m_pCallBackParas(para)
{

}

void COfflineMsgTask::run()
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();
	if (m_callBack)
	{
		m_callBack(m_pTaskData, m_pCallBackParas);
	}
	//DbgLog("thread %lu run COfflineMsgTask %p, use %llu usecond\r\n", pthread_self(), this, elspsedTimer.elapsed());
}


/////////////////////////////////////////////////////////////////////

COfflineMsgTaskMgr::~COfflineMsgTaskMgr()
{

}

int COfflineMsgTaskMgr::Init(CConfigFileReader* pReader)
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
	log("COfflineMsgTaskMgr created %d threads", common_thread_num);

	m_bHasInit = true;
	return 0;
}

CImThreadPool* COfflineMsgTaskMgr::GetCommonThreadPoolInstance()
{
	return m_pCommonThreadPool;
}

bool COfflineMsgTaskMgr::InsertOfflineMsgTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, offlineMsgTaskCallBack callBack, void* para, int index)
{
	//����task,��task��ִ�в��붯������ɺ�ص�
	COfflineMsgTask* pInsertTask = new COfflineMsgTask(pGrpTask, callBack, para);
	
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

void COfflineMsgTaskMgr::Stop()
{
	if (m_pCommonThreadPool)
	{
		m_pCommonThreadPool->StopThreads();
		m_pCommonThreadPool->waitForDone();		//���̵߳ȴ����е�mongoThread����
	}
}

COfflineMsgTaskMgr::COfflineMsgTaskMgr()
{

}

CImThreadPool* COfflineMsgTaskMgr::m_pCommonThreadPool = NULL;

bool COfflineMsgTaskMgr::m_bHasInit = false;
