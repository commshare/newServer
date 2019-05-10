/******************************************************************************
Filename: locationInfoMgr.cpp
Description: 
******************************************************************************/
#include "mysqlPool.h"
#include "acl_cpp/lib_acl.hpp"
#include "custInfoMgr.h"

#define TABLE_NAME_WAITERS			                "`waiters`"
#define TABLE_NAME_SERVICE_QUESTIONS                "`service_questions`"
#define TABLE_NAME_SERVICE_QUESTIONS_BIND           "`service_question_to_waiters`"

//TABLE_NAME_WAITERS 表字段
#define WAITERS_FIELD_SERVICEID            "`service_id`"
#define WAITERS_FIELD_USERID               "`user_id`"

//TABLE_NAME_SERVICE_QUESTIONS_BIND 表字段
#define SERVICE_QUESTIONS_BIND_FILED_WAITERID             "`waiter_id`"
#define SERVICE_QUESTIONS_BIND_FILED_ID             "`id`"

//TABLE_NAME_SERVICE_QUESTIONS 表字段
#define SERVICE_QUESTIONS_FIELD_ID   				   "`id`"
#define SERVICE_QUESTIONS_FIELD_ROOTID   		   "`root_id`"



CCustInfoMgr::CCustInfoMgr()
{

}

CCustInfoMgr::~CCustInfoMgr()
{

}

bool CCustInfoMgr::getCustListByServiceId(const string& serviceId,CustList& custList)
{
	bool ret = false;
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		ErrLog("get mysql connection for mysql failed");
		return ret;
	}

	string strSql = "select distinct " + string(WAITERS_FIELD_USERID) + " from " + TABLE_NAME_WAITERS 
					+ " where " + WAITERS_FIELD_SERVICEID + "= :;";

	acl::query query1;
	query1.create_sql(strSql.c_str())
	.set_parameter(WAITERS_FIELD_SERVICEID, serviceId.c_str());

	DbgLog("%s", query1.to_string().c_str());
	if (pConn->exec_select(query1) == false)
	{
		ErrLog("sql (%s) error", query1.to_string().c_str());
		return ret;
	}

	const acl::db_rows* result1 = pConn->get_result();
	if (result1)
	{
		int num = result1->length();

		for(int i=0; i<num; i++)
		{
			const acl::db_row* pRow = result1->get_rows()[i];
	
			const acl::db_row& row = *pRow;
			custList.push_back(row[size_t(0)]);		
		}
		ret = true;
	}
	else
	{
		WarnLog("serviceId[%s] have no cust",serviceId.c_str());
	}

	pConn->free_result();
	return ret;
	
}


bool CCustInfoMgr::getCustListByQuestionId(const string& questionId,const string& serviceId,CustList& custList)
{
	bool ret = false;
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		ErrLog("get mysql connection for mysql failed");
		return ret;
	}

	string strSql="";
	
	//如果没有问题ID,则获取服务号下的所有客服列表
	if(questionId.empty())
	{
		return getCustListByServiceId(serviceId,custList);
	}
	else 
	{
		//获取问题ID绑定的客服列表
		ret = getCustListByQuestionId(questionId,custList);

		if(true == ret)
		{
			return ret;
		}
		else
		{	//问题ID没有绑定客服，则返回客服号下的客服列表
			return getCustListByServiceId(serviceId,custList);
		}
	}


}

bool CCustInfoMgr::getCustListByQuestionId(const string& questionId,CustList& custList)
{
	bool ret = false;
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		ErrLog("get mysql connection for mysql failed");
		return ret;
	}

	string strSql = "select distinct " + string(SERVICE_QUESTIONS_BIND_FILED_WAITERID) + " from " + TABLE_NAME_SERVICE_QUESTIONS_BIND 
					+ " where " + SERVICE_QUESTIONS_BIND_FILED_ID + " in ("
					+ "select " + SERVICE_QUESTIONS_FIELD_ROOTID + " from " + TABLE_NAME_SERVICE_QUESTIONS 
					+ " where " + SERVICE_QUESTIONS_FIELD_ID + "= :" + ");" ;

	acl::query query1;
	query1.create_sql(strSql.c_str())
	.set_parameter(SERVICE_QUESTIONS_FIELD_ID, questionId.c_str());

	DbgLog("%s", query1.to_string().c_str());
	if (pConn->exec_select(query1) == false)
	{
		ErrLog("sql (%s) error", query1.to_string().c_str());
		return ret;
	}

	const acl::db_rows* result1 = pConn->get_result();
	if (result1)
	{
		int num = result1->length();

		for(int i=0; i<num; i++)
		{
			const acl::db_row* pRow = result1->get_rows()[i];
	
			const acl::db_row& row = *pRow;
			custList.push_back(row[size_t(0)]);		
		}
		ret = true;
	}
	else
	{
		WarnLog("questionId[%s] have no cust",questionId.c_str());
	}

	pConn->free_result();
	return ret;

}




