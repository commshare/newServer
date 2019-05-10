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

typedef void(*offlineMsgTaskCallBack)(const std::shared_ptr<::google::protobuf::MessageLite>& pOfflineMsg, void* data);				// 

class  COfflineMsgTask :public CTask
{
public:
	COfflineMsgTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, offlineMsgTaskCallBack callBack, void* para);
	virtual ~COfflineMsgTask(){};
	std::shared_ptr<::google::protobuf::MessageLite> GetDataEntry()const{ return m_pTaskData; }

	virtual void run() override;

private:
	std::shared_ptr<::google::protobuf::MessageLite> m_pTaskData;		//Ҫ�����������Ϣ
	offlineMsgTaskCallBack m_callBack = NULL;
	void * m_pCallBackParas = NULL;
};

class COfflineMsgTaskMgr :public Singleton < COfflineMsgTaskMgr >
{
public:
	virtual ~COfflineMsgTaskMgr();

	//��ȡ�����ļ�������dbServer�������Ϣ������������Ϣ����dbPool����
	int Init(CConfigFileReader* pReader);

	static CImThreadPool* GetCommonThreadPoolInstance();
	static bool InsertOfflineMsgTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, offlineMsgTaskCallBack callBack, void* para, int index = -1);
	void Stop();
private:

	friend class Singleton < COfflineMsgTaskMgr >;
	COfflineMsgTaskMgr();

private:
	static bool m_bHasInit;
	static CImThreadPool* m_pCommonThreadPool;
};




#endif // __COMMONTASKMGR_H__


