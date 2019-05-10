#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/database.hpp>
#include "threadpool.h"
#include "mongoDbManager.h"
#include "mongoTask.h"
#include "util.h"
#include <bsoncxx/json.hpp>
#include <json/json.h>
#include "channelMongoOperator.h"

using mongocxx::client;
using mongocxx::collection;
using std::string;

using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

const string adminDbName = "admin";

ChannelMongoOperator ::ChannelMongoOperator()
	: m_sAdminDbName(adminDbName)
	, m_sCurrDbName("")
{

}

ChannelMongoOperator::~ChannelMongoOperator()
{

}

void ChannelMongoOperator::initCurrDbName(const string& dbName)
{
	m_sCurrDbName = dbName;
	m_mongoCollections = GetCollections();
}

void ChannelMongoOperator::checkAndCreateCollection(const std::string& collectionName)
{
	auto it =  m_mongoCollections.find(collectionName);//local determine
	if(it == m_mongoCollections.end())
	{
		if(!HasCollection(collectionName)) //check real mongoDb for distributed system;
		{
			CreateCollection(collectionName);
			CreateIndex(collectionName, "msgId", true);
		}
		m_mongoCollections.insert(collectionName);
	}
}

bool ChannelMongoOperator::CreateCollection(const std::string& CollectionName)
{
    std::unique_ptr<CMongoDbConn> pConn =
        CMongoDbManager::getInstance()->GetDBConn();

    if (!pConn)
    {
        WarnLog("conn invalied");
        return false;
    }

    string newCollection = m_sCurrDbName+ ".";
    newCollection += CollectionName;

    auto shardcollectionbuilder= bsoncxx::builder::stream::document{};
    bsoncxx::document::value  createSdCollection = shardcollectionbuilder
        << "shardCollection"<< newCollection.c_str()
        <<  "key" << bsoncxx::builder::stream::open_document << "msgId" << "hashed"<< bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize;

    try
    {
        auto database= pConn->GetDatabase(m_sAdminDbName);
        auto ret =  database.run_command(createSdCollection.view());
        return true;
    }
    catch (const std::exception& xcp)
    {
        WarnLog("exception catched:%s", xcp.what());
    }

    return false;
}

bool ChannelMongoOperator::DelCollection(const string& collectionName)
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
        auto database= pConn->GetDatabase(m_sCurrDbName);
        database[collectionName].drop();
        return true;
    }
    catch (const std::exception& xcp)
    {
        WarnLog("exception catched:%s", xcp.what());
    }

    return false;
}

std::set<string> ChannelMongoOperator::GetCollections()
{
    
    std::unique_ptr<CMongoDbConn> pConn =
        CMongoDbManager::getInstance()->GetDBConn();

    if (!pConn)
    {
        WarnLog("conn invalied");
        return set<string>();
    }


    try
    {
//        auto ret =  pConn->GetDatabase(m_sCurrDbName).list_collection_names();
//        return ret;
         set<string> vcollections;
         mongocxx::cursor cursor = pConn->GetDatabase(m_sCurrDbName).list_collections();
         for (auto&& doc : cursor)                                                     
         {                                                
            string collName =  doc["name"].get_utf8().value.to_string();
            vcollections.insert(collName);
         }

         return vcollections;
    }
    catch (const std::exception& xcp)
    {
        WarnLog("exception catched:%s", xcp.what());
    }

    return set<string>();

}

bool ChannelMongoOperator::CreateIndex(const string& collectionName, const string& indexName, bool unique, bool backGround)
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
        auto result = pConn->GetCollection(m_sCurrDbName, collectionName).create_index(index_specification.view(), indexOption);
        return true;
    }
    catch (const std::exception& xcp)
    {   
        WarnLog("exception catched:%s", xcp.what());
    }

    return false;
}

bool ChannelMongoOperator::IsExistCollection(string& collectionName)
{
    set<string> vColls =  GetCollections();

//    for_each(vColls.begin(), vColls.end, collectionName);
    bool ret = false;
  //  for(auto it = vColls.begin(); it != vColls.end(); it++) 
  //  {
  //      if(collectionName.compare(*it) == 0) 
  //      {
  //          ret = true;
  //          break;
  //      }
  //  }
    auto it = vColls.find(collectionName);
    ret = (it != vColls.end());
    return ret;
}

bool ChannelMongoOperator::HasCollection(const string& collectionName)
{

    std::unique_ptr<CMongoDbConn> pConn =
        CMongoDbManager::getInstance()->GetDBConn();

    bool ret = false;
    if (!pConn)
    {
        WarnLog("conn invalied");
        return ret; 
    }

    try
    {
        ret = pConn->GetDatabase(m_sCurrDbName).has_collection(collectionName);
        return ret;
    }
    catch (const std::exception& xcp)
    {
        WarnLog("check whether has collection error : %s", xcp.what());
        return ret;
    }

}

string ChannelMongoOperator::InsertChannelOfflineMsg(const CChannelOfflineMsg& offlineMsg, const string& collectionName)    
{
    std::unique_ptr<CMongoDbConn> pConn =
        CMongoDbManager::getInstance()->GetDBConn();

    string str("");
    if (!pConn)
    {
        WarnLog("conn invalied");
        return str; 
    }

    if (!offlineMsg.IsValid())
    {
        WarnLog("data invalid");
        return str;
    }

    try
    {
//        mongocxx::options::update updateOption;
//        updateOption.upsert(true);

//        bsoncxx::stdx::optional< mongocxx::result::replace_one > result =
//            pConn->GetCollection(m_sCurrDbName, collectionName).replace_one(offlineMsg.KeyDoc().view(), offlineMsg.ToDoc().view(), updateOption);
        //log("insert mongo data %s", bsoncxx::to_json(mongoData.ToDoc().view()).c_str());

       bsoncxx::stdx::optional< mongocxx::result::insert_one > result =
           pConn->GetCollection(m_sCurrDbName, collectionName).insert_one(offlineMsg.ToDoc().view());
        if (result)
        {
           // auto tmp = const_cast<bsoncxx::types::b_document*>(&result->inserted_id().get_document())->view();
           // str = tmp["_id"].get_utf8().value.to_string();
            //str = result->inserted_id().get_utf8().value.to_string();
            //str = result->inserted_id().get_document()["_id"].get_utf8().value.to_string();
            str = result->inserted_id().get_oid().value.to_string();
            return str;
        }
        else
        {
            WarnLog("insert mongo data %s failed", bsoncxx::to_json(offlineMsg.ToDoc().view()).c_str());
            return string("");
        }
    }
    catch (const std::exception& xcp)
    {
        WarnLog("Insert offlineMsg failed, exception catched:%s", xcp.what());
		string strError = xcp.what();
		if(0 == strError.find("E11000"))  // 重复插入问题
		{
			return "E11000";
		}
        //here to create collection
        return "" ;
    }
}

int ChannelMongoOperator::UpdateChannelOfflineMsg(const std::string& collectionName, const std::string& msgId, const std::string& extend)
{
	std::unique_ptr<CMongoDbConn> pConn = CMongoDbManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		WarnLog("conn invalied");
		return -1; 
	}

	try
	{
		bsoncxx::builder::basic::document key{};
		key.append( kvp("msgId", msgId) );
		
		bsoncxx::builder::basic::document doc{};
		doc.append( /*kvp("extend", extend),*/
					kvp("$inc", [](sub_document subdoc) {subdoc.append(kvp("isCancel", 1));}));
	
		bsoncxx::stdx::optional< mongocxx::result::update > result = 
						pConn->GetCollection(m_sCurrDbName, collectionName).update_one(key.view(), doc.view());
		if (result)
		{
			return result->modified_count();
		}
		else
		{
		    WarnLog("update mongo data %s failed", bsoncxx::to_json(key.view()).c_str());
		    return -1;
		}
	}
	catch (const std::exception& xcp)
	{
	    WarnLog("update offlineMsg failed, exception catched:%s", xcp.what());
	    return -1;
	}
}

bool ChannelMongoOperator::getChannelOfflineMsg(const std::string& collectionName, std::vector<std::string>& vecResult, int limitNum)
{
	std::unique_ptr<CMongoDbConn> pConn = CMongoDbManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		WarnLog("conn invalied");
		return false;
	}

	try
	{
		bsoncxx::builder::basic::document doc{};
		mongocxx::options::find findOption{};
		if (limitNum > 0)
		{
			findOption.limit(limitNum);
		}
		//查询条件
		bsoncxx::builder::basic::document sortDoc{};
		sortDoc.append(kvp("_id", -1));
		findOption.sort(sortDoc.view());
	
		mongocxx::cursor cursor = pConn->GetCollection(m_sCurrDbName, collectionName).find(doc.view(), findOption);
		for (auto&& view : cursor)
		{
			vecResult.emplace_back(bsoncxx::to_json(view));
		}
	}
	catch (const std::exception& xcp)
	{
		WarnLog("get offlineMsg failed, exception catched:%s", xcp.what());
		return false;
	}
}

string ChannelMongoOperator::GetChannelOfflineMsg(const std::string& collectionName, const std::string& msgId)
{
	std::unique_ptr<CMongoDbConn> pConn = CMongoDbManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		WarnLog("conn invalied");
		return "";
	}

	try
	{
		bsoncxx::builder::basic::document key{};
		key.append( kvp("msgId", msgId) );
		bsoncxx::stdx::optional<bsoncxx::document::value> result = pConn->GetCollection(m_sCurrDbName, collectionName).find_one(key.view());
		if (result)
		{
			Json::Value msgValue;
			bsoncxx::document::view value = result->view();
			if(value["_id"])
			{
				msgValue["id"] = value["_id"].get_oid().value.to_string();
			}
			if(value["fromId"])
			{
				msgValue["fromId"] = value["fromId"].get_utf8().value.to_string();
			}if(value["radioId"])
			{
				msgValue["radioId"] = value["radioId"].get_utf8().value.to_string();
			}
			if(value["msgId"])
			{
				msgValue["msgId"] = value["msgId"].get_utf8().value.to_string();
			}
			if(value["msgTime"])
			{
				msgValue["msgTime"] = (long long)value["msgTime"].get_int64();
			}
			if(value["encrypt"])
			{
				msgValue["encrypt"] = (int)value["encrypt"].get_int64();
			}
			if(value["content"])
			{
				msgValue["content"] = value["content"].get_utf8().value.to_string();
			}
			if(value["extend"])
			{
				msgValue["extend"] = value["extend"].get_utf8().value.to_string();
			}
			//return bsoncxx::to_json(*result);
			return msgValue.toStyledString();
			
		}
		else
		{
			WarnLog("find mongo data %s failed", bsoncxx::to_json(key.view()).c_str());
			return "";
		}
	}
	catch (const std::exception& xcp)
	{
		WarnLog("find offlineMsg failed, exception catched:%s", xcp.what());
		return "";
	}
}



