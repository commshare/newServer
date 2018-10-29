/******************************************************************************
Filename: mongoTask.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/05
Description: 
******************************************************************************/
#include "mongoTask.h"
#include "util.h"


CMongoTask::CMongoTask(const CMongoDbColl& coll, void* pCallBackParas)
	: m_pCallBackParas(pCallBackParas), m_dbConn(coll)
{

}

/********************************************************************************/

CMongoInsertTask::CMongoInsertTask(const std::shared_ptr<IMongoDataEntry>& pMongoData, const CMongoDbColl& coll,
			mongoInsertCallBack callBack, void* pCallBackParas)
	: CMongoTask(coll, pCallBackParas), m_pMongoData(pMongoData), m_callBack(callBack)
{

}

CMongoInsertTask::CMongoInsertTask(const std::shared_ptr<IMongoDataEntry>& pMongoData, const CMongoDbColl& coll,
            mongoInsertCallBack__ callBack, void* pCallBackParas)
    : CMongoTask(coll, pCallBackParas), m_pMongoData(pMongoData), m_callBack__(callBack)
{

}

void CMongoInsertTask::run()
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

    unsigned short bInsertSuccess = m_dbConn.InsertOne(*m_pMongoData, 0xffff);

	DbgLog("thread %lu insert %p's data to db use %llu usecond", pthread_self(), this, elspsedTimer.elapsed());
    if (m_callBack)//callback for(bool bInsertSuccess, unsigned short to bool is compacity)
    {
        m_callBack(m_pMongoData, bInsertSuccess, m_pCallBackParas);
    }

    if (m_callBack__) //callback for (unsigned short bInsertSuccess)
	{
        m_callBack__(m_pMongoData, bInsertSuccess, m_pCallBackParas);
	}
	DbgLog("thread %lu run CMongoInsertTask %p callback, use %llu usecond\r\n", pthread_self(), this, elspsedTimer.elapsed());
}

/*********************************************************************************/
//
//CMongoNoOperTask::CMongoNoOperTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, mongoNoOperCallBack callBack, void* para)
//	: m_pTaskData(pGrpTask), m_callBack(callBack), m_pCallBackParas(para)
//{
//
//}
//
//void CMongoNoOperTask::run()
//{
//	CUsecElspsedTimer elspsedTimer;
//	elspsedTimer.start();
//	if (m_callBack)
//	{
//		m_callBack(m_pTaskData, m_pCallBackParas);
//	}
//	DbgLog("thread %lu run CMongoNoOperTask %p, use %llu usecond\r\n", pthread_self(), this, elspsedTimer.elapsed());
//}

/********************************************************************************/

CMongoInsertManyTask::CMongoInsertManyTask(const std::vector<std::shared_ptr<IMongoDataEntry> >& mongoDatas, const CMongoDbColl& coll, mongoInsertManyCallBack callBack, void* pCallBackParas)
	: CMongoTask(coll, pCallBackParas), m_mongoDatas(mongoDatas), m_callBack(callBack)
{

}

void CMongoInsertManyTask::run()
{
	DbgLog("thread %lu  start run insert task %p", pthread_self(),this);
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();
	bool bInsertSuccess = m_dbConn.InsertMany(m_mongoDatas);
	DbgLog("thread %lu insert %p's data to db use %llu usecond,list Size = %d", pthread_self(), this, elspsedTimer.elapsed(), m_mongoDatas.size());
	if (m_callBack)
	{
		m_callBack(m_mongoDatas, bInsertSuccess, m_pCallBackParas);
	}
	DbgLog("thread %lu run CMongoInsertManyTask %p, use %llu usecond\r\n", pthread_self(), this, elspsedTimer.elapsed());
}
 
/********************************************************************************/

CMongoDelTask::CMongoDelTask(const std::shared_ptr<IMongoDataDelKeys>& pKey, const CMongoDbColl& coll, mongoDelCallBack callBack, void* pCallBackParas)
	: CMongoTask(coll, pCallBackParas), m_pKey(pKey), m_callBack(callBack)
{

}

void CMongoDelTask::run()
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();
	bool bDelSuccess = m_dbConn.DelOne(*m_pKey);
	if (m_callBack)
	{
		m_callBack(m_pKey, bDelSuccess, m_pCallBackParas);
	}
	DbgLog("thread %lu run  CMongoDelTask %p, use %llu usecond ", pthread_self(), this, elspsedTimer.elapsed());
}

CMongoUpdateTask::CMongoUpdateTask(const std::shared_ptr<IMongoDataDelKeys>& pKey, const std::shared_ptr<IMongoDataEntry>& pMongoData, const CMongoDbColl& coll, mongoInsertCallBack callBack, void* pCallBackParas)
:CMongoTask(coll, pCallBackParas), m_pKey(pKey), m_pMongoData(pMongoData), m_callBack(callBack)
{

}

void CMongoUpdateTask::run()
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();
	bool bUpdateSuccess = m_dbConn.UpdateOne(*m_pKey, *m_pMongoData);
	if (m_callBack)
	{
		m_callBack(m_pMongoData, bUpdateSuccess, m_pCallBackParas);
	}
	DbgLog("thread %lu run  CMongoUpdateTask %p, use %llu usecond ", pthread_self(), this, elspsedTimer.elapsed());
}
