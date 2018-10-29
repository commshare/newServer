/******************************************************************************
Filename: mongoDbColl.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/15
Description: 
******************************************************************************/

#ifndef __MONGODBCOLL_H__
#define __MONGODBCOLL_H__

#include <memory>
#include <vector>
#include <string>
#include <bsoncxx/builder/basic/document.hpp>

//
//namespace google
//{
//	namespace protobuf
//	{
//		class MessageLite;
//	}
//}


class IMongoDataEntry
{
public:
	//转换成doc保存到
	//IMongoDataEntry(const bsoncxx::document::view& doc){}
	virtual bsoncxx::builder::basic::document ToDoc()const = 0;
	virtual bsoncxx::builder::basic::document KeyDoc()const = 0;
	virtual std::shared_ptr<IMongoDataEntry> Clone()const = 0;
	virtual bool IsValid()const{ return false; }
	//在异步插入时根据hasVal决定放入到哪个线程
	virtual unsigned int hashVal()const{ return (unsigned int)(- 1); }
};

class IMongoDataDelKeys
{
public:
	virtual bsoncxx::builder::basic::document ToDoc()const = 0;
	virtual std::shared_ptr<IMongoDataDelKeys> Clone()const = 0;
	//在异步插入时根据hasVal决定放入到哪个线程
	virtual unsigned int hashVal()const{ return (unsigned int)(-1); } 
};

typedef void(*mongoInsertManyCallBack)(const std::vector<std::shared_ptr<IMongoDataEntry> >& pOfflineMsgs, bool result, void* data);				// 
typedef void(*mongoInsertCallBack)(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, bool result, void* data);				// 
typedef void(*mongoInsertCallBack__)(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, unsigned short result, void* data);
typedef void(*mongoDelCallBack)(const std::shared_ptr<IMongoDataDelKeys>& pDelKey, bool result, void* data);					// 

//typedef void(*mongoNoOperCallBack)(const std::shared_ptr<::google::protobuf::MessageLite>& pOfflineMsg, void* data);				// 
class CMongoInsertTask;
class CMongoDelTask;
class CMongoUpdateTask;

enum MONGO_OPERATION_STATE {
    MONGO_OPERATION_FAILE,
    MONGO_OPERATION_INSERT_ONE= 0x0001,
    MONGO_OPERATION_REPLACE_ONE = 0x0002,
    MONGO_OPERATION_DELETE_ONE = 0x0004,
    MONGO_OPERATION_INSERT_MANY = 0x0008,
    MONGO_OPERATION_DELETE_MANY = 0x0010,
    MONGO_OPERATION_SUCCESS = 0x0020,
    MONGO_OPERATION_RESERVE
};

class CMongoDbColl 
{
public:
	CMongoDbColl(const std::string& dbName, const std::string& collName);
	virtual ~CMongoDbColl();
	
	bool CreateIndex(const std::string& indexName, bool unique = false, bool backGround = true);
	bool CreateIndex(const std::vector<std::string>& indexNames);
	bool InsertOne(const IMongoDataEntry& mongoData);
    unsigned short InsertOne(const IMongoDataEntry& mongoData, int);
    CMongoInsertTask* InsertOne(const IMongoDataEntry& mongoData, mongoInsertCallBack callBack, void* para)const;
    CMongoInsertTask* InsertOne(const IMongoDataEntry& mongoData, mongoInsertCallBack__ callBack, void* para)const;
	//bool InsertOne(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, mongoNoOperCallBack callBack, void* para);
	bool InsertMany(const std::vector<std::shared_ptr<IMongoDataEntry> >& mongoDatas);
	bool InsertMany(const std::vector<std::shared_ptr<IMongoDataEntry> >& mongoDatas, mongoInsertManyCallBack callBack, void* para)const;
	bool DelOne(const IMongoDataDelKeys& Key);
	CMongoDelTask* DelOne(const IMongoDataDelKeys& pKey, mongoDelCallBack callBack, void* para)const;

	bool DelMany(const IMongoDataDelKeys& Key);

	//更新记录
	bool UpdateOne(const IMongoDataDelKeys& Key, const IMongoDataEntry& mongoData);
	CMongoUpdateTask* UpdateOne(const IMongoDataDelKeys& key, const IMongoDataEntry& mongoData, mongoInsertCallBack callBack, void* para)const;

	const std::string& GetDbName()const{ return m_sDbName; }
	const std::string& GetCollName()const{ return m_sCollName; }

private:
	void SetDbName(const std::string& dbName){ m_sDbName = dbName; }
	void SetCollectionName(const std::string& collName){ m_sCollName = collName; }

private:
	std::string m_sDbName;								//database name
	std::string m_sCollName;								//collection name
};

#endif // __MONGODBCOLL_H__


