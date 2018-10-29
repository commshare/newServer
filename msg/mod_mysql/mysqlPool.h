/******************************************************************************
Filename: mysqlPool.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/16
Description: 
******************************************************************************/
#ifndef __MYSQLPOOL_H__
#define __MYSQLPOOL_H__

#include <map>
#include <memory>
#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "singleton.h"
#include "util.h"

class CDBPool;

class CDBConn 
{
public:
	CDBConn(acl::db_handle* Handle, CDBPool* pDBPool);
	virtual ~CDBConn();
	//int Init();

	//bool tbl_exists(const char* tbl_name);
	bool sql_select(const char* sql, acl::db_rows* result = NULL);
	bool sql_update(const char* sql);
	/**
	* 更安全易用的查询过程，调用此函数功能等同于 sql_select，只是查询
	* 对象 query 构建的 sql 语句是安全的，可以防止 sql 注入，该方法
	* 执行 SELECT SQL 语句
	* @param query {query&}
	* @param result {db_rows*} 如果非空，则将查询结果填充进该结果对象中，
	*  否则，会引用 db_handle 内部的一个临时存储对象
	* @return {bool} 执行是否成功
	*/
	bool exec_select(acl::query& query, acl::db_rows* result = NULL);

	/**
	* 更安全易用的更新过程，调用此函数功能等同于 sql_update，只是查询
	* 对象 query 构建的 sql 语句是安全的，可以防止 sql 注入，该方法
	* 执行 INSERT/UPDATE/DELETE SQL 语句
	* @param query {query&}
	* @return {bool} 执行是否成功
	*/
	bool exec_update(acl::query& query);

	const acl::db_rows* get_result() const;
	void free_result();

	const char* GetPoolName();
	acl::db_handle* GetHandle() { return m_pHandle; }
private:
	acl::db_handle* 		m_pHandle;	// realHandle
	CDBPool* 				m_pDBPool;	// to get MySQL server information
};

class CDBPool 
{
public:
	CDBPool(const char* pool_name, const char* db_server_ip, uint16_t db_server_port,
		const char* username, const char* password, const char* db_name, int max_conn_cnt);
	~CDBPool();


	int Init();
	/**
	* 设置连接池中空闲连接的空闲生存周期
	* @param ttl {time_t} 空闲连接的生存周期，当该值 < 0 则表示空闲连接不过期，
	*  == 0 时表示立刻过期，> 0 表示空闲该时间段后将被释放
	* @return {connect_pool&}
	*/
	CDBPool& set_idle_ttl(time_t ttl);

	std::unique_ptr<CDBConn> GetDBConn();

	const char* GetPoolName() { return m_sPoolName.c_str(); }
	const char* GetDBServerIP() { return m_sDbServerIp.c_str(); }
	uint16_t GetDBServerPort() { return m_nDbServerPort; }
	const char* GetUsername() { return m_sUserName.c_str(); }
	const char* GetPasswrod() { return m_sPassword.c_str(); }
	const char* GetDBName() { return m_sDbName.c_str(); }

	acl::db_pool* GetRealPool()const { return m_pDBPool;}
private:
	uint16_t	m_nDbServerPort;
	int 		m_nDbMaxConnCnt;	
	acl::db_pool* m_pDBPool = NULL;

	string 		m_sPoolName;
	string 		m_sDbServerIp;
	string 		m_sUserName;
	string 		m_sPassword;
	string 		m_sDbName;
};

// manage db pool (master for write and slave for read)
class CDBManager :public Singleton < CDBManager >
{
public:
	virtual ~CDBManager();

	/* should call Init at first time */
	int Init(const string& configFriendName);

	std::unique_ptr<CDBConn> GetDBConn(const string& dbpool_name="");
private:
	CDBManager();
	friend class Singleton < CDBManager >;

private:
	map<string, CDBPool*>	m_dbpoolMap;
};

#endif