/******************************************************************************
Filename: mongoDbColl.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/15
Description: 
******************************************************************************/


#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include "threadpool.h"
#include "mongoDbColl.h"
#include "mongoDbManager.h"
#include "mongoTask.h"
#include "util.h"
#include <bsoncxx/json.hpp>

using mongocxx::client;
using mongocxx::collection;
using std::string;

using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;


CMongoDbColl::CMongoDbColl(const string& dbName, const string& collName)
	: m_sDbName(dbName), m_sCollName(collName)
{
	
}


CMongoDbColl::~CMongoDbColl()
{

}

bool CMongoDbColl::CreateIndex(const string& indexName, bool unique, bool backGround)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("conn invalied");
		return false;
	}

	auto index_specification = document{} << indexName << 1 << finalize;
	mongocxx::options::index indexOption;
	indexOption.unique(unique);
	indexOption.background(backGround);

	try
	{
		auto result = pConn->GetCollection(m_sDbName, m_sCollName).create_index(index_specification.view(), indexOption);
		return true;
	}
	catch (const std::exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
	}

	return false;
}

bool CMongoDbColl::CreateIndex(const std::vector<string>& indexNames)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("conn invalied");
		return false;
	}

	bsoncxx::builder::basic::document index_specification{};

	uint32_t index = 0;
	for (index = 0; index < indexNames.size(); ++index)
	{
		index_specification.append(kvp(indexNames[index], 1));
	}

	mongocxx::options::index indexOption;
	indexOption.unique(true);
	indexOption.background(true);

	try
	{
		auto result = pConn->GetCollection(m_sDbName, m_sCollName).create_index(index_specification.view(), indexOption);
		return true;
	}
	catch (const std::exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
	}

	return false;
}


bool CMongoDbColl::InsertOne(const IMongoDataEntry& mongoData)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("conn invalied");
		return false;
	}

	if (!mongoData.IsValid())
	{
		WarnLog("data invalid");
		return false;
	}

	try
	{
		//bsoncxx::stdx::optional< mongocxx::result::insert_one > result =
		//	pConn->GetCollection(m_sDbName, m_sCollName).insert_one(mongoData.ToDoc().view());
		 

		mongocxx::options::update updateOption;
		updateOption.upsert(true);

		bsoncxx::stdx::optional< mongocxx::result::replace_one > result =
			pConn->GetCollection(m_sDbName, m_sCollName).replace_one(mongoData.KeyDoc().view(), mongoData.ToDoc().view(), updateOption);
		//log("insert mongo data %s", bsoncxx::to_json(mongoData.ToDoc().view()).c_str());
        
		if (result /*&& result->result().inserted_count() > 0*/)
		{
			return true;
		}
		else
		{
			WarnLog("insert mongo data %s failed", bsoncxx::to_json(mongoData.ToDoc().view()).c_str());
		}
		return false;
	}
	catch (const std::exception& xcp)
	{
		WarnLog("Insert offlineMsg failed, exception catched:%s", xcp.what());
		return false;
	}

    return false;
}

unsigned short CMongoDbColl::InsertOne(const IMongoDataEntry &mongoData, int)
{
    std::unique_ptr<CMongoDbConn> pConn =
        CMongoDbManager::getInstance()->GetDBConn();

    if (!pConn)
    {
        WarnLog("conn invalied");
        return false;
    }

    if (!mongoData.IsValid())
    {
        WarnLog("data invalid");
        return false;
    }

    unsigned short Ret = 0;
    try
    {
        //bsoncxx::stdx::optional< mongocxx::result::insert_one > result =
        //	pConn->GetCollection(m_sDbName, m_sCollName).insert_one(mongoData.ToDoc().view());


        mongocxx::options::update updateOption;
        updateOption.upsert(true);

        bsoncxx::stdx::optional< mongocxx::result::replace_one > result =
            pConn->GetCollection(m_sDbName, m_sCollName).replace_one(mongoData.KeyDoc().view(), mongoData.ToDoc().view(), updateOption);
        //log("insert mongo data %s", bsoncxx::to_json(mongoData.ToDoc().view()).c_str());
        if (result && result->result().matched_count() > 0) //成功，并重复插入了
        {
            Ret |= MONGO_OPERATION_SUCCESS;
            return Ret |= MONGO_OPERATION_REPLACE_ONE;
        }

        if(result && result->result().matched_count() == 0) { //成功，第一次插入了

            return Ret |= MONGO_OPERATION_SUCCESS;
        } else {
            WarnLog("insert mongo data %s failed", bsoncxx::to_json(mongoData.ToDoc().view()).c_str());
        }
    }
    catch (const std::exception& xcp)
    {
        WarnLog("Insert offlineMsg failed, exception catched:%s", xcp.what());
        return Ret;
    }

    return Ret;
}


CMongoInsertTask* CMongoDbColl::InsertOne(const IMongoDataEntry& mongoData, mongoInsertCallBack callBack, void* para)const
{
	//����task,��task��ִ�в��붯�������ɺ��ص�
	CMongoInsertTask* pInsertTask = new CMongoInsertTask(mongoData.Clone(), *this, callBack, para);

	CImThreadPool* pThreadPool = CMongoDbManager::GetMongoThreadPoolInstance();
	if (NULL == pInsertTask || NULL == pThreadPool)	//��������ʧ�ܣ����ûص�����
	{
		if (callBack)
		{
			callBack(mongoData.Clone(), false, para);
		}
		return NULL;
	}
	pThreadPool->AddTask(pInsertTask, /*-1*/ mongoData.hashVal());
	return pInsertTask;
}

CMongoInsertTask* CMongoDbColl::InsertOne(const IMongoDataEntry& mongoData, mongoInsertCallBack__ callBack, void* para)const
{
    //����task,��task��ִ�в��붯�������ɺ��ص�
    CMongoInsertTask* pInsertTask = new CMongoInsertTask(mongoData.Clone(), *this, callBack, para);

    CImThreadPool* pThreadPool = CMongoDbManager::GetMongoThreadPoolInstance();
    if (NULL == pInsertTask || NULL == pThreadPool)	//��������ʧ�ܣ����ûص�����
    {
        if (callBack)
        {
            callBack(mongoData.Clone(), 0, para);
        }
        return NULL;
    }
    pThreadPool->AddTask(pInsertTask, /*-1*/ mongoData.hashVal());
    return pInsertTask;
}

//bool CMongoDbColl::InsertOne(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, mongoNoOperCallBack callBack, void* para)
//{
//	//����task,��task��ִ�в��붯�������ɺ��ص�
//	CMongoNoOperTask* pInsertTask = new CMongoNoOperTask(pGrpTask, callBack, para);
//
//	CImThreadPool* pThreadPool = CMongoDbManager::GetMongoThreadPoolInstance();
//	if (NULL == pInsertTask || NULL == pThreadPool)	//��������ʧ�ܣ����ûص�����
//	{
//		if (callBack)
//		{
//			callBack(pGrpTask, para);
//		}
//		return false;
//	}
//	pThreadPool->AddTask(pInsertTask);
//	return true;
//}


bool CMongoDbColl::InsertMany(const std::vector<std::shared_ptr<IMongoDataEntry> >& mongoDatas)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("conn invalied");
		return false;
	}

	const int MAXMSGNUM = 1000;
	int loops = (mongoDatas.size()+MAXMSGNUM - 1) / MAXMSGNUM;

	int curLoop = 0;
	for (curLoop = 0; curLoop < loops; ++curLoop)
	{
		uint32_t index = 0;

		vector<bsoncxx::document::value> documents;
		for (index = 0; index < MAXMSGNUM && (index + curLoop * MAXMSGNUM) < mongoDatas.size(); ++index)
		{
			const std::shared_ptr<IMongoDataEntry>& pOfflineMsg = mongoDatas[index + curLoop * MAXMSGNUM];
			documents.push_back(pOfflineMsg->ToDoc().extract());
		}

		try
		{
			bsoncxx::stdx::optional< mongocxx::result::insert_many > result 
				= pConn->GetCollection(m_sDbName, m_sCollName).insert_many(documents);
		}
		catch (const exception& xcp)
		{
			WarnLog("exception catched:%s", xcp.what());
			return false;
		}
	}
	return true;
}
bool CMongoDbColl::InsertMany(const std::vector<std::shared_ptr<IMongoDataEntry> >& mongoDatas, mongoInsertManyCallBack callBack, void* para) const
{
	//����task,��task��ִ�в��붯�������ɺ��ص�
	CMongoInsertManyTask* pInsertTask = new CMongoInsertManyTask(mongoDatas, *this, callBack, para);

	CImThreadPool* pThreadPool = CMongoDbManager::GetMongoThreadPoolInstance();
	if (NULL == pInsertTask || NULL == pThreadPool)	//��������ʧ�ܣ����ûص�����
	{
		if (callBack)
		{
			callBack(mongoDatas, false, para);
		}
		if (pInsertTask)
		{
			delete pInsertTask;
		}
		return false;
	}
	pThreadPool->AddTask(pInsertTask);
	return true;
}




bool CMongoDbColl::DelOne(const IMongoDataDelKeys& key)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("conn invalied");
		return false;
	}
	
	try
	{
		static bsoncxx::builder::basic::document doc{};
		static bool bUpdateIndex = false;
		//static document doc{};
		if (!bUpdateIndex)
		{
			doc.append(kvp("$inc",
				[](sub_document subdoc) {
				subdoc.append(kvp("bPulled", 1));
			}));
			bUpdateIndex = true;
		}

		/*if (pConn->GetCollection(m_sDbName, m_sCollName).delete_many(key.ToDoc().view()))*/
		if (pConn->GetCollection(m_sDbName, m_sCollName).update_many(key.ToDoc().view(), doc.view()))
		{
			//DbgLog("delete mongo data %s success", bsoncxx::to_json(key.ToDoc().view()).c_str());
			return true;
		}
		else
		{
			WarnLog("delete mongo data %s failed", bsoncxx::to_json(key.ToDoc().view()).c_str());
		}
	}
	catch (const exception& xcp)
	{
		WarnLog("delete msg failed, exception catched:%s", xcp.what());
	}

	return false;
}

CMongoDelTask* CMongoDbColl::DelOne(const IMongoDataDelKeys& key, mongoDelCallBack callBack, void* para) const
{
	//����task,��task��ִ�в��붯�������ɺ��ص�
	CMongoDelTask* pDelTask = new CMongoDelTask(key.Clone(), *this, callBack, para);
	CImThreadPool* pThreadPool = CMongoDbManager::GetMongoThreadPoolInstance();
	if (NULL == pDelTask || NULL == pThreadPool)	//��������ʧ�ܣ����ûص�����
	{
		if (callBack)
		{
			callBack(key.Clone(), false, para);
		}
		return NULL;
	}
	pThreadPool->AddTask(pDelTask, /*-1*/key.hashVal());
	return pDelTask;
}

bool CMongoDbColl::UpdateOne(const IMongoDataDelKeys& key, const IMongoDataEntry& mongoData)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("conn invalied");
		return false;
	}

	try
	{
		log("update mongo key %s", bsoncxx::to_json(key.ToDoc().view()).c_str());
		//log("update mongo key %s",  bsoncxx::to_json(mongoData.ToDoc().view()).c_str());
		mongocxx::options::update updateOption;
		updateOption.upsert(true);

		bsoncxx::stdx::optional< mongocxx::result::replace_one > result =
			pConn->GetCollection(m_sDbName, m_sCollName).replace_one(key.ToDoc().view(), mongoData.ToDoc().view(), updateOption);

		/*if (pConn->GetCollection(m_sDbName, m_sCollName).update_many(key.ToDoc().view(), mongoData.ToDoc().view()))*/
		if (result)
		{
			return true;
		}
		else
		{
			WarnLog("update mongo data %s failed", bsoncxx::to_json(key.ToDoc().view()).c_str());
		}
	}
	catch (const exception& xcp)
	{
		WarnLog("update msg failed, exception catched:%s", xcp.what());
	}

	return false;
}

CMongoUpdateTask* CMongoDbColl::UpdateOne(const IMongoDataDelKeys& key, const IMongoDataEntry& mongoData, mongoInsertCallBack callBack, void* para)const
{
	//����task,��task��ִ�в��붯�������ɺ��ص�
	CMongoUpdateTask* pUpdateTask = new CMongoUpdateTask(key.Clone(),mongoData.Clone(), *this, callBack, para);
	CImThreadPool* pThreadPool = CMongoDbManager::GetMongoThreadPoolInstance();
	if (NULL == pUpdateTask || NULL == pThreadPool)	//��������ʧ�ܣ����ûص�����
	{
		if (callBack)
		{
			callBack(mongoData.Clone(), false, para);
		}
		return NULL;
	}
	pThreadPool->AddTask(pUpdateTask, /*-1*/key.hashVal());
	return pUpdateTask;
}

bool CMongoDbColl::DelMany(const IMongoDataDelKeys& Key)
{
	return DelOne(Key);
}
