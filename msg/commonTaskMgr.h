/******************************************************************************
Filename: commonTaskMgr.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/10/08
Description: 
******************************************************************************/
#ifndef __COMMONTASKMGR_H__
#define __COMMONTASKMGR_H__

#include <memory>
#include "task.h"
#include "singleton.h"
//#include "util.h"
//#include "impdubase.h"

class CConfigFileReader;
class CImThreadPool;


namespace google
{
	namespace protobuf
	{
		class MessageLite;
	}
}

typedef void(*commobTaskCallBack)(const std::shared_ptr<::google::protobuf::MessageLite>& pOfflineMsg, void* data);				// 
//typedef void(*msgHandleTaskCallBack)(const std::shared_ptr<CImPdu>& pPdu, void* data);

class  CCommonTask :public CTask
{
public:
	CCommonTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, commobTaskCallBack callBack, void* para);
	virtual ~CCommonTask(){};
	std::shared_ptr<::google::protobuf::MessageLite> GetDataEntry()const{ return m_pTaskData; }

	virtual void run() override;

private:
	std::shared_ptr<::google::protobuf::MessageLite> m_pTaskData;		//Ҫ�����������Ϣ
	commobTaskCallBack m_callBack = NULL;
	void * m_pCallBackParas = NULL;
};

//class  CMsgHandleTask :public CTask
//{
//public:
//	CMsgHandleTask(const std::shared_ptr<CImPdu>& pPdu, msgHandleTaskCallBack callBack, void* para);
//	virtual ~CMsgHandleTask(){};
//	std::shared_ptr<CImPdu> GetDataEntry()const{ return m_pTaskData; }
//
//	virtual void run() override;
//
//private:
//	std::shared_ptr<CImPdu> m_pTaskData;		//Ҫ�����������Ϣ
//	msgHandleTaskCallBack m_callBack = NULL;
//	void * m_pCallBackParas = NULL;
//};


class CCommonTaskMgr :public Singleton < CCommonTaskMgr >
{
public:
	virtual ~CCommonTaskMgr();

	//��ȡ�����ļ�������dbServer�������Ϣ������������Ϣ����dbPool����
	int Init(CConfigFileReader* pReader);

	static CImThreadPool* GetCommonThreadPoolInstance();
	static bool InsertCommonTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, commobTaskCallBack callBack, void* para, int index = -1);
	//static bool InsertMsgHandleTask(const std::shared_ptr<CImPdu>& pPdu, msgHandleTaskCallBack callBack, void* para);
	void Stop();
private:

	friend class Singleton < CCommonTaskMgr >;
	CCommonTaskMgr();

private:
	static bool m_bHasInit;
	static CImThreadPool* m_pCommonThreadPool;
};




#endif // __COMMONTASKMGR_H__


