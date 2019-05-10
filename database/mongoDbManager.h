/******************************************************************************
Filename: mongoDbManager.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/15
Description: 
******************************************************************************/
#ifndef __MONGODBMANAGER_H__
#define __MONGODBMANAGER_H__

#include <memory>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <string>
#include "singleton.h"


using std::string;
using mongocxx::client;

class CMongoDbConn
{
public:
	CMongoDbConn(std::unique_ptr< client, std::function< void(client *)>> client);
	CMongoDbConn(CMongoDbConn && conn);
	virtual ~CMongoDbConn();

	mongocxx::collection GetCollection(const string& dbName, const string& collName)const;
	mongocxx::database GetDatabase(const string& dbName)const;
private:
	CMongoDbConn(const CMongoDbConn&);				//declare copy constructor but not defined, forbidden use it
	CMongoDbConn& operator=(const CMongoDbConn&);	//declare Assignment operator but not defined, forbidden use it

private:
	std::unique_ptr< client, std::function< void(client *)>> mRealClient;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMongoDbPool 
{
public:
	CMongoDbPool(const char* pool_name, const char* db_server_ip, uint16_t db_server_port = 27017,
		const char* username="", const char* password="", int min_conn_cnt = 10, int max_conn_cnt = 10);
	virtual ~CMongoDbPool();

	std::unique_ptr<CMongoDbConn> GetDBConn();

	const char* GetPoolName()		{ return m_sPoolName.c_str(); }
	const char* GetDBServerIP()		{ return m_sServerIp.c_str(); }
	uint16_t	GetDBServerPort()	{ return m_nServerPort; }
	const char* GetUsername()		{ return m_sUserName.c_str(); }
	const char* GetPasswrod()		{ return m_sPassword.c_str(); }
private:
	static mongocxx::uri generateUri(const char* db_server_ip, uint16_t db_server_port,
		const char* username, const char* password, int min_conn_cnt, int max_conn_cnt);
private:
	uint16_t	m_nServerPort;
	int 		m_nMinConnCnt;		//最小连接数
	int 		m_nMaxConnCnt;		//最大连接数
	string 		m_sPoolName;
	string 		m_sServerIp;	
	string 		m_sUserName;
	string 		m_sPassword;
	mongocxx::pool m_RealPool;
};

///////////////////////////////////////////////////////////////////////////////////////

class CConfigFileReader;
class CImThreadPool;

// manage mongodb pool
class CMongoDbManager :public Singleton < CMongoDbManager >
{
public:
	virtual ~CMongoDbManager();

	//读取配置文件，加载dbServer的相关信息并根据配置信息创建dbPool对象
	int Init(CConfigFileReader* pReader, bool initThreadPool = true);

	static CImThreadPool* GetMongoThreadPoolInstance();
	std::unique_ptr<CMongoDbConn> GetDBConn();
	void Stop();
private:

	CMongoDbPool* getDBConnPool()const;
	friend class Singleton < CMongoDbManager > ;
	CMongoDbManager();

private:
	std::map<string, CMongoDbPool*>	m_dbpoolMap;
	mongocxx::instance m_inst;			//驱动
	static bool m_bHasInit;
	static CImThreadPool* m_pMongoThreadPool;
};


#endif // __MONGODBMANAGER_H__
