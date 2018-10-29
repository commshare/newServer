#include "mysqlhelper.h"


CMysqlHelper::CMysqlHelper()
	: m_pMysqlPool(NULL)
{
}

CMysqlHelper::~CMysqlHelper()
{
	if (!m_pMysqlPool)
	{
		delete m_pMysqlPool;
	}
}

bool CMysqlHelper::Initialize(CConfigFileReader* pConfigReader)
{
	m_pConfigReader =  pConfigReader;

	if(!SetMysqlHelper())
		return false;

	/**
	 * 采用 mysql 数据库时的构造函数
	 * @param dbaddr {const char*} mysql 服务器地址，格式：IP:PORT，
	 *  在 UNIX 平台下可以为 UNIX 域套接口
	 * @param dbname {const char*} 数据库名
	 * @param dbuser {const char*} 数据库用户
	 * @param dbpass {const char*} 数据库用户密码
	 * @param dblimit {int} 数据库连接池的最大连接数限制
	 * @param dbflags {unsigned long} mysql 标记位
	 * @param auto_commit {bool} 是否自动提交
	 * @param conn_timeout {int} 连接数据库超时时间(秒)
	 * @param rw_timeout {int} 与数据库通信时的IO时间(秒)
	 * @param charset {const char*} 连接数据库的字符集(utf8, gbk, ...)
	 */
	
	m_pMysqlPool = new acl::mysql_pool(m_sMysqlUrl.c_str(), m_sDbName.c_str(), m_sDbUser.c_str(), m_sDbPass.c_str());
	if (!m_pMysqlPool)
	{
		ErrLog("Failed to connect mysql = %s",m_sMysqlUrl.c_str());
		return false;
	}


	return true;
}

bool CMysqlHelper::SetMysqlHelper(void)
{
	if(!m_pConfigReader)
		return false;

	char* pMysql = m_pConfigReader->GetConfigName("mysql_host");
	if(pMysql==NULL)
		return false;
	m_sMysqlUrl = pMysql;

	pMysql = m_pConfigReader->GetConfigName("mysql_datebase");
	if(pMysql==NULL)
		return false;
	m_sDbName = pMysql;

	pMysql = m_pConfigReader->GetConfigName("mysql_username");
	if(pMysql==NULL)
		return false;
	m_sDbUser = pMysql;

	pMysql = m_pConfigReader->GetConfigName("mysql_password");
	if(pMysql==NULL)
		return false;
	m_sDbPass = pMysql;

	//pMysql = m_pConfigReader->GetConfigName("mysql_poolsize");
	//if(pMysql==NULL)
	//	return false;
	//m_nPoolSize = atoi(pMysql);

	return true;
}


bool CMysqlHelper::GetMemberStatus(acl::db_handle* hDB, const char *sSql,uint8_t &bStatus)
{
	if(!hDB || !sSql)
		return false;

	if (hDB->sql_select(sSql) == false)
	{
		ErrLog("Failed to select sql: %s", sSql);
		bStatus = 0xff;
		hDB->free_result();
		
		return false;
	
	}	

	
	const acl::db_rows* result = hDB->get_result();

	
	if (result)
	{
	
		const std::vector<acl::db_row*>& rows = result->get_rows();
		//printf("row size = %ld \n",rows.size());
		for (size_t i = 0; i < rows.size(); i++)
		{
			const acl::db_row* row = rows[i];
			//if ((*row)[MEMBERSTATUS_POS] != NULL)
			//{
			//	bStatus = atoi((*row)[MEMBERSTATUS_POS]);
			//	break;
			//}

			bStatus = row->field_int("MEMBER_STATUS",0xff);
			break;
		}

	}	

	hDB->free_result();
	return true;
}

bool CMysqlHelper::GetUserPermission(acl::db_handle* hDB,const char *sSql,uint8_t &bPermit)
{
	if(!hDB || !sSql)
		return false;

	if (hDB->sql_select(sSql) == false)
	{
		ErrLog("Failed to select sql: %s", sSql);
		bPermit = 0xff;
		hDB->free_result();
		
		return false;
	
	}	


	const acl::db_rows* result = hDB->get_result();

	
	if (result)
	{
	
		const std::vector<acl::db_row*>& rows = result->get_rows();
		//printf("row size = %ld \n",rows.size());
		for (size_t i = 0; i < rows.size(); i++)
		{
			const acl::db_row* row = rows[i];
			//if ((*row)[USERPERMISSION_POS] != NULL)
			//{
			//	bPermit = atoi((*row)[USERPERMISSION_POS]);
			//	break;
			//}
			bPermit = row->field_int("in_group_allow",0xff);
			break;
		}

	}	

	hDB->free_result();
	return true;

}


bool CMysqlHelper::GetGroupNumber(acl::db_handle* hDB,const char *sSql,uint16_t &nNumber)
{
	log("------------------------get group number sql:%s", sSql);
	if(!hDB || !sSql)
		return false;

	if (hDB->sql_select(sSql) == false)
	{
		ErrLog("Failed to select sql: %s", sSql);
		nNumber = 0;
		hDB->free_result();
		
		return false;
	
	}	

	
	const acl::db_rows* result = hDB->get_result();
	if (result)
	{

		const std::vector<acl::db_row*>& rows = result->get_rows();
		printf("row size = %ld \n",rows.size());
		
		const acl::db_row* row = rows[0];
		if (row != NULL)
		{
			nNumber = row->field_int((size_t) 0, (int) 0);
			return true;
		}
	}

	/*
		const std::vector<acl::db_row*>& rows = result->get_rows();
		if(rows.size() > 0)
		{
			const acl::db_row* row = rows[0];
			if((*row)[0] != NULL)
			{
				//nNumber = row->field_int(0,(int)0);
				return true;
			}
		}

	}
	*/

	return false;
}

#if 0
bool CMysqlHelper::GetGroupMaster(acl::db_handle* hDB,const char *sSql,string &sMasterId,uint8_t &bPermit)
{
	if (hDB->sql_select(sSql) == false)
	{
		ErrLog("Failed to select sql: %s", sSql);
		hDB->free_result();
		return false;
	}	
	
		
	const acl::db_rows* result = hDB->get_result();
	
	if (result)
	{
		const std::vector<acl::db_row*>& rows = result->get_rows();
		for (size_t i = 0; i < rows.size(); i++)
		{
			const acl::db_row* row = rows[i];
			if ((*row)[3] != NULL && (*row)[6] != NULL)
			{
				bPermit = atoi((*row)[3]);
				sMasterId = (*row)[6];
				break;
			}
		}
	
	}	

	DbgLog("sql : %s,master %s,permit %d",sSql,sMasterId.c_str(),bPermit);
	hDB->free_result();	
	return true;
}
#endif

bool CMysqlHelper::GetGroupInfo(acl::db_handle* hDB, const string& sGroupId, GroupInfo_t &info)
{
	if(!hDB)
		return false;
	
	string sInfoSql = "select GROUP_ID,GROUP_COUNT,GROUP_PERMISSION,GROUP_STATUS,GROUP_MASTERID,UNIX_TIMESTAMP(GROUP_CREATETIME) from group_t where GROUP_ID = '" + sGroupId + "'";

	if (hDB->sql_select(sInfoSql.c_str()) == false)
	{
		ErrLog("Failed to select sql: %s", sInfoSql.c_str());
		hDB->free_result();
		return false;
	}


	const acl::db_rows* result = hDB->get_result();

	if (result)
	{
		const std::vector<acl::db_row*>& rows = result->get_rows();
		for (size_t i = 0; i < rows.size(); i++)
		{
			const acl::db_row* row = rows[i];
			if ((*row)[1] != NULL && (*row)[4] != NULL)
			{
				info.sGroupId = (*row)[(size_t)0];
				info.nGroupNumber = atoi((*row)[1]);
				info.bPermit = atoi((*row)[2]);
				info.bStatus = atoi((*row)[3]);
				info.sMasterId = (*row)[4];
				info.createTime = atoi((*row)[5]);
				//bPermit = atoi((*row)[3]);
				//sMasterId = (*row)[6];
				break;
			}
		}
	
	}	

	DbgLog("sql : %s,Group %s,master %s,GroupCount %d", sInfoSql.c_str(), info.sGroupId.c_str(),
		info.sMasterId.c_str(),info.nGroupNumber);
	
	hDB->free_result();	
	return true;
}


bool CMysqlHelper::Insert(acl::db_handle* hDB,const char*  sSql)
{
	if(!hDB || !sSql)
		return false;

	
	log("--------------begin insert data to database  sql:%s-------------------", sSql);
	if (hDB->sql_update(sSql) == false)
	{
		ErrLog("Failed to insert sql: %s",sSql);
		hDB->free_result();
		return false;
	}

	log("--------------end insert data to database-------------------");
	
	hDB->free_result();
	return true;
}


bool CMysqlHelper::Update(acl::db_handle* hDB,const char*  sSql)
{
	if(!hDB || !sSql)
		return false;

	log("--------------begin update data to database  sql:%s-------------------", sSql);
	if (hDB->sql_update(sSql) == false)
	{
		ErrLog("Failed toupdate sql: %s, %s",sSql, hDB->get_error());
		hDB->free_result();
		
		return false;
	}
	log("--------------end update data to database-------------------");
	hDB->free_result();
	return true;
}



