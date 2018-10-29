/******************************************************************************
Filename: mongoTask.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/05
Description: 
******************************************************************************/
#ifndef __MONGOTASK_H__
#define __MONGOTASK_H__
#include <memory>
#include "task.h"
#include "mongoDbColl.h"


class CMongoTask :public CTask
{
public:
	CMongoTask(const CMongoDbColl& coll, void* pCallBackParas);
	virtual ~CMongoTask(){}

protected:
	void * m_pCallBackParas = NULL;
	CMongoDbColl m_dbConn;
};


class  CMongoInsertTask :public CMongoTask
{
public:
	CMongoInsertTask(const std::shared_ptr<IMongoDataEntry>& pMongoData, const CMongoDbColl& coll, mongoInsertCallBack callBack, void* pCallBackParas);
    CMongoInsertTask(const std::shared_ptr<IMongoDataEntry>& pMongoData, const CMongoDbColl& coll, mongoInsertCallBack__ callBack, void* pCallBackParas);
	virtual ~CMongoInsertTask(){};
	std::shared_ptr<IMongoDataEntry> GetDataEntry()const{ return m_pMongoData; }

	virtual void run() override;

private:
	std::shared_ptr<IMongoDataEntry> m_pMongoData;		//要保存的离线消息
	mongoInsertCallBack m_callBack = NULL;
    mongoInsertCallBack__ m_callBack__ = NULL;
};

//class  CMongoNoOperTask :public CTask
//{
//public:
//	CMongoNoOperTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, mongoNoOperCallBack callBack, void* para);
//	virtual ~CMongoNoOperTask(){};
//	std::shared_ptr<::google::protobuf::MessageLite> GetDataEntry()const{ return m_pTaskData; }
//
//	virtual void run() override;
//
//private:
//	std::shared_ptr<::google::protobuf::MessageLite> m_pTaskData;		//要保存的离线消息
//	mongoNoOperCallBack m_callBack = NULL;
//	void * m_pCallBackParas = NULL;
//};

class  CMongoInsertManyTask :public CMongoTask
{
public:
	CMongoInsertManyTask(const std::vector<std::shared_ptr<IMongoDataEntry> >& mongoDatas, const CMongoDbColl& coll, mongoInsertManyCallBack callBack, void* pCallBackParas);
	virtual ~CMongoInsertManyTask(){};
	const std::vector<std::shared_ptr<IMongoDataEntry> >& GetDataEntries()const{ return m_mongoDatas; }

	virtual void run() override;

private:
	const std::vector<std::shared_ptr<IMongoDataEntry> > m_mongoDatas;		//要保存的离线消息
	mongoInsertManyCallBack m_callBack = NULL;
};


class  CMongoDelTask :public CMongoTask
{
public:
	//CMongoDelTask(MongoInsertCallBack callBack, CPacket* handler, void* paras);
	CMongoDelTask(const std::shared_ptr<IMongoDataDelKeys>& pKey, const CMongoDbColl& coll, mongoDelCallBack callBack, void* pCallBackParas);
	virtual ~CMongoDelTask(){}
	//const std::string& GetUserId()const{ return m_userId; }

	virtual void run() override;

private:
	//要删除的消息
	std::shared_ptr<IMongoDataDelKeys> m_pKey;
	mongoDelCallBack m_callBack;
};

class  CMongoUpdateTask :public CMongoTask
{
public:
	CMongoUpdateTask(const std::shared_ptr<IMongoDataDelKeys>& pKey, const std::shared_ptr<IMongoDataEntry>& pMongoData, const CMongoDbColl& coll, mongoInsertCallBack callBack, void* pCallBackParas);
	virtual ~CMongoUpdateTask(){}

	virtual void run() override;

private:
	//要更新的的信息
	std::shared_ptr<IMongoDataDelKeys> m_pKey;
	std::shared_ptr<IMongoDataEntry> m_pMongoData;		//要保存的离线消息
	mongoInsertCallBack m_callBack;
};

#endif // __MONGOTASK_H__
