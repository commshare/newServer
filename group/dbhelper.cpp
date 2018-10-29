#include "dbhelper.h"

static const ErrCode _GroupPermissionCode[3] = 
{
	ERR_GROUP_NONPERMISSION,			//直接加入
	ERR_GROUP_INTERPERMISSION,			//需要允许
	ERR_GROUP_FORBIDDEN					//禁止加入
};
	
CDbHelper::CDbHelper()
	: m_pThreadPool(NULL),m_pMysqlPool(NULL),m_nPoolSize(0)
{
}

CDbHelper::~CDbHelper()
{
	if(!m_pThreadPool)
	{
		delete m_pThreadPool;
	}
}

bool CDbHelper::Initialize(CConfigFileReader* pConfigReader)
{
	if(!pConfigReader)
		return false;


	char *pPoolSize = pConfigReader->GetConfigName("database_poolsize");
	if(pPoolSize==NULL)
		return false;
	m_nPoolSize = atoi(pPoolSize);
		m_pThreadPool = new CImThreadPool();

	if(m_pThreadPool->Init(m_nPoolSize))
	{
		ErrLog("Encounter exception error when initiate thread pool!");
		return false;
	}

	m_pMysqlPool = CMysqlHelper::GetInstance()->GetPool();
	m_pRedisCluster = CRedisHelper::GetInstance()->GetCluster();
	
	return true;
}


bool CDbHelper::DatabaseHelper(SqlAccess_t &sqlAccess)
{
	if(!m_pMysqlPool|| !sqlAccess.accessCb || !sqlAccess.pAccessPacket)
	{
		ErrLog("Failed to invoke database helper");
		return false;
	}
	
	sqlAccess.pAccessSqlPool = m_pMysqlPool; 
	sqlAccess.pAccessRedisCluster = m_pRedisCluster;

	if(m_pThreadPool)
	{
		log("--------------DatabaseHelper-------------------");
		
    	CTask* pTask = new CDbTask(sqlAccess);
     	m_pThreadPool->AddTask(pTask);	
	}
	return true;

}

CDbTask::CDbTask(SqlAccess_t &sqlAccess)
{
	m_pPacketProcess = sqlAccess.pAccessPacket;
	m_accessCb = sqlAccess.accessCb;
	m_accessTask = sqlAccess.accessTask;
	m_pAccessPara = sqlAccess.pAccessPara;
	m_pAccessSqlPool = sqlAccess.pAccessSqlPool;
	m_pAccessRedisCluster = sqlAccess.pAccessRedisCluster;
	m_hashRedis.set_cluster(m_pAccessRedisCluster,100); // default 100
	m_redis.set_cluster(m_pAccessRedisCluster, 100);

	m_hDB = m_pAccessSqlPool->peek_open();
}

CDbTask::~CDbTask()
{
	m_pAccessSqlPool->put(m_hDB);
}

ErrCode CDbTask::IsMember(string sUserId,string sGroupId,uint8_t &bMemberStatus)
{
	string sGroupKey = PREFIX_GRPMEMBERCACHE;
	sGroupKey += sGroupId;
	uint8_t bStatus = 0xff;
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;
	
	if(!CRedisHelper::GetInstance()->GetMemberStatus(m_hashRedis,sGroupKey.c_str(),sUserId.c_str(),bStatus))
	{
		string sMemberSql = "select MEMBER_STATUS from groupmember_t where GROUP_ID = '" + sGroupId + "' and MEMBER_ID = '" + sUserId + "'";
		if(!CMysqlHelper::GetInstance()->GetMemberStatus(m_hDB, sMemberSql.c_str(),bStatus) || bStatus==0xff || bStatus == 0x0)
		{
			
			DbgLog("the user %s, status %d. isn't group %s member.",sUserId.c_str(),bStatus,sGroupId.c_str());	
			errCode = ERR_GROUP_MEMBERNONEXIST;
			bMemberStatus = bStatus;
			
			return	errCode ;
		}
	}

	//DbgLog("the group %s user %s, status %d. It should be abnormal member if status is 0 !",
	//		sGroupId.c_str(),sUserId.c_str(),bStatus);
	//errCode = !bStatus ? ERR_GROUP_MEMBERNONEXIST : ERR_GROUP_MEMBEREXIST;
	errCode = ERR_GROUP_MEMBEREXIST;
	bMemberStatus = bStatus;

	return errCode;
	
}

ErrCode CDbTask::IsMaster(string sUserId,string sGroupId)
{
#if 1
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;
	GroupInfo_t info;

	info.sGroupId =  sGroupId.c_str();

	if(GetGroupInfo(info))
	{
		DbgLog("user %s compare with group master %s",sUserId.c_str(),info.sMasterId.c_str());
		errCode = (!sUserId.compare(info.sMasterId)) ? ERR_GROUP_MASTER : ERR_GROUP_NOTMASTER;	
	}

	return errCode;
	
#else
	string sGroupCacheId = PREFIX_GRPCACHE;
	string sMasterId="";
	uint8_t bPermit = 0xff;
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;
	
	sGroupCacheId += sGroupId;
	
	if(!CRedisHelper::GetInstance()->GetGroupMaster(m_hashRedis,sGroupCacheId.c_str(),sMasterId,bPermit))
	{
		string sMasterSql = "select * from group_t where GROUP_ID = '" + sGroupId + "'";
		if(!CMysqlHelper::GetInstance()->GetGroupMaster(m_hDB, sMasterSql.c_str(),sMasterId,bPermit))
		{
			DbgLog("Failed to access mysql when reading group table.");
			return 	errCode ;
		}
	}

	errCode = (!sUserId.compare(sMasterId)) ? ERR_GROUP_MASTER : ERR_GROUP_NOTMASTER;
	
	return errCode;
#endif

}

ErrCode CDbTask::VerifyGroupMasterPermission(string sUserId, string sGroupId)
{
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;
	
#if 1
	GroupInfo_t info;

	info.sGroupId = sGroupId.c_str();
	if(GetGroupInfo(info))
	{
		DbgLog("Group %s permission %d",sGroupId.c_str(),info.bPermit);
		if(info.bStatus != NORMAL_GROUPSTATUS)
			errCode = ERR_GROUP_FORBIDDEN;
			
		else
		{
			//errCode = (info.bPermit>=0 && info.bPermit<=2) ?
			errCode = _GroupPermissionCode[info.bPermit];
		}
		m_pAccessPara->sMasterId = info.sMasterId;
	}
#else
	string sGroupCacheId = PREFIX_GRPCACHE;
	string sMasterId;
	uint8_t bPermit = 0xff;
		
	sGroupCacheId += sGroupId;
	
	if(!CRedisHelper::GetInstance()->GetGroupMaster(m_hashRedis,sGroupCacheId.c_str(),sMasterId,bPermit))
	{
		string sPermitSql = "select * from group_t where GROUP_ID = '" + sGroupId + "'";
		if(!CMysqlHelper::GetInstance()->GetGroupMaster(m_hDB, sPermitSql.c_str(),sMasterId,bPermit))
		{
			DbgLog("Failed to access mysql when reading group table.");
			return 	errCode ;
		}
	}
	
	if(!bPermit)
		errCode = ERR_GROUP_NONPERMISSION;
	else if(bPermit==1)
		errCode = ERR_GROUP_INTERPERMISSION;
	else if(bPermit==2)
		errCode = ERR_GROUP_FORBIDDEN;
	
	m_pAccessPara->sMasterId = sMasterId;
	
#endif	

	return errCode;
}

ErrCode CDbTask::VerifyGroupUserPermission(string sUserId, uint8_t &bGroupPermit)
{
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;
	uint8_t bPermit = 0xff;

	string sUserSql = "select in_group_allow from users where userid = '" + sUserId +"'";
	if(!CMysqlHelper::GetInstance()->GetUserPermission(m_hDB, sUserSql.c_str(),bPermit) || bPermit==0xff)
	{
		
		DbgLog("Fail to check user %s about group permission.",sUserId.c_str());	
		//errCode = ERR_GROUP_MEMBERNONEXIST;
		
		//bMemberStatus = bStatus;
		
		return	errCode ;
	}
	DbgLog("user %s permission %d",sUserId.c_str(),bPermit);
	bGroupPermit = bPermit;
	
	return NON_ERR;
}

//ErrCode CDbTask::VerifyGroupPermission(string sUserId,string sGroupId)
//{
//	return (!m_pAccessPara->bMode) ? VerifyGroupMasterPermission(sUserId,sGroupId) :
//						VerifyGroupUserPermission(sUserId,sGroupId);
//}

ErrCode CDbTask::VerifyGroupCreatingAuth(string sUserId, uint8_t bRole, uint16_t nGroupLimit)
{
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;

	string sCountSql = "select count(*) from group_t where GROUP_MASTERID = '" + sUserId + "' and (GROUP_STATUS = 1 OR  (GROUP_STATUS = 2 AND GROUP_CREATETIME >= DATE_SUB(NOW(),INTERVAL 10 MINUTE)))";
	uint16_t nNumber = 0;

	if(CMysqlHelper::GetInstance()->GetGroupNumber(m_hDB, sCountSql.c_str(),nNumber))
	{
		DbgLog("User %s has aleady create group total is %d",sUserId.c_str(),nNumber);
		errCode = (nNumber>=nGroupLimit) ? ERR_GROUP_OVERCREATION : NON_ERR;
	}

	return errCode;
}

ErrCode CDbTask::VerifyGroupJoiningAuth(string sGroupId, uint8_t bRole, uint16_t nMemberLimit,uint16_t nJoinNum, uint32_t appyType)
{
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;

	do
	{
		GroupInfo_t info;
		info.sGroupId =  sGroupId.c_str();
		if(!GetGroupInfo(info))
			break;
		if (DISMISS_GROUPSTATUS == info.bStatus)
		{
			errCode = ERR_GROUP_DISMISSED;
			break;
		}
		else if(NORMAL_GROUPSTATUS == info.bStatus || DEPOSIT_GROUPSTATUS == info.bStatus)
		{
			DbgLog("group join auth apply type %d", appyType);
			if(appyType == FACE2FACE_APPY)
			{
				//十分钟内未激活的可以申请加入，十分钟以上的不能加入
				if(getCurrentTime() / 1000 - info.createTime >= 10 * 60)
				{
					errCode = ERR_GROUP_FORBIDDEN;
					break;
				}
			}
			// 对群人数进行判断
			errCode = ((info.nGroupNumber + nJoinNum) >= nMemberLimit) ? ERR_GROUP_OVERJOIN : NON_ERR;
		}

	}while(0);

	return errCode;
}


void CDbTask::GenerateGroupId(string& sGroupId,string sAppId)
{
#if 1	
	UidCode_t uid;
	string sUid;
	//char rBuf[64] = {0};
	char sBuf[64] = {0};

	GenerateUId(uid);
	toHexLower(sUid, (const void*)&uid,UID_SIZE);
	sprintf(sBuf,"%s",sUid.c_str());

	//pUid = sBuf;

	//j = 0;
	//for(i = 0; i < UID_SIZE * 2; i++)
	//{
	//	//DbgLog("uid code: %x",sBuf[i]);
	//	if(sBuf[i] >= 0x30 && sBuf[i]<= 0x39)
	//	{
	//		rBuf[j] = sBuf[i];
	//		++j;
	//	}
	//}
	
#else 
	static unsigned int inc = 0;
	char tbuf[64] = {0};

	sprintf(tbuf,"%d%d",(unsigned)time(0),inc++);
	sGroupId = tbuf;
#endif
	
	sGroupId = sAppId + sBuf;
}



bool CDbTask::GetGroupInfo(GroupInfo_t &info)
{
	if(info.sGroupId=="")
		return false;
	string sGroupId = info.sGroupId.c_str();
	string sGroupKey = PREFIX_GRPCACHE;
	sGroupKey += info.sGroupId.c_str();
	
	if(!CRedisHelper::GetInstance()->GetGroupInfo(m_hashRedis,sGroupKey.c_str(),info))
	{
		/*string sInfoSql = "select * from group_t where GROUP_ID = '" + sGroupId + "'";*/
		
		if (!CMysqlHelper::GetInstance()->GetGroupInfo(m_hDB, sGroupId, info))
		{
			ErrLog("Failed to read group %s information!", info.sGroupId.c_str());
			return false;
		}
		
	}

	return true;
}


ErrCode CDbTask::UpdateGroupStatus(string sGroupId,uint8_t bStatus)
{
	string sStatus = int2string(bStatus); 	
	string sUpdateGroupSql;

	if ( DISMISS_GROUPSTATUS == bStatus)
		sUpdateGroupSql = "update group_t set GROUP_STATUS = '" + sStatus + 
							 "',GROUP_UPDATETIME=now() where GROUP_ID = '"+sGroupId +"'and GROUP_COUNT=0";
	else
		sUpdateGroupSql = "update group_t set GROUP_STATUS = '" + sStatus +
					"',GROUP_UPDATETIME=now() where GROUP_ID = '" + sGroupId + "'";
	

	CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateGroupSql.c_str());

	DbgLog("update grp %s state to %d", sGroupId.c_str(), bStatus);

	return NON_ERR;
}

ErrCode CDbTask::UpdateGroupCount(string sGroupId, uint16_t nCount,uint8_t bMethod)
{
	ErrCode nResult =	ERR_GROUP_NETWORKEXCEPTION;
	string sqlMemberCount = "select count(*) from groupmember_t where group_id='" + sGroupId + "' and member_status != 0;";
	uint16_t nNumber = 0;
	string sUpdateGroupSql = "";
	if(CMysqlHelper::GetInstance()->GetGroupNumber(m_hDB, sqlMemberCount.c_str(),nNumber))
	{
		sUpdateGroupSql = "update group_t set group_count=" + int2string(nNumber) + " where group_id='" + sGroupId + "';";
	}
	else 
	{
		string sCount = int2string(nCount);
		string sMethod = !bMethod  ? "+" : "-";

		sUpdateGroupSql = "update group_t set GROUP_COUNT = GROUP_COUNT " + sMethod + "'"+ sCount + 
				"',GROUP_UPDATETIME=now() where GROUP_ID = '"+sGroupId +"'and GROUP_COUNT > 0";	
	}

	if(CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateGroupSql.c_str()))
	{
		string sGroupKey = PREFIX_GRPCACHE;
		sGroupKey += sGroupId;
		CRedisHelper::GetInstance()->UpdateGroupCount(m_hashRedis,m_redis,sGroupKey.c_str(),nCount,bMethod);
		UpdateGroupStatus(sGroupId,DISMISS_GROUPSTATUS);  //dismiss group in actively only group count equals to 0 
		nResult = NON_ERR;
	}

	return nResult;
}

ErrCode CDbTask::UpdateGroupMember(string sUserId,string sGroupId,uint8_t bStatus)
{
	ErrCode errCode  =	ERR_GROUP_NETWORKEXCEPTION;
	string sMemberStatus = int2string(bStatus);
	
	string sUpdateMemberSql = "update groupmember_t set MEMBER_STATUS = '" + sMemberStatus + 
		"',MEMBER_UPDATETIME=now() where GROUP_ID = '"+	sGroupId + "' and MEMBER_ID = '" + sUserId + "'";

	if(CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateMemberSql.c_str()))
	{
		string sGroupKey = PREFIX_GRPMEMBERCACHE;
		sGroupKey += sGroupId;
		
		CRedisHelper::GetInstance()->InsertGroupMember(m_hashRedis,sGroupKey.c_str(),sUserId.c_str(),bStatus);
		UpdateGroupCount(sGroupId,1);
		errCode = NON_ERR;
	}

	return errCode;	
	
}

ErrCode CDbTask::InsertGroup(GroupCreate* pCreateInst,string sAppId)
{
	const uint64_t currentTime = getCurrentTime()/1000;
	ErrCode nResult =	ERR_GROUP_NETWORKEXCEPTION;
	const char* sCreateSql = "insert into group_t(GROUP_ID, GROUP_NAME,GROUP_AVATAR,GROUP_COUNT,GROUP_PERMISSION,GROUP_STATUS,GROUP_ANNOUNCEMENT,GROUP_MASTERID,GROUP_CREATETIME,GROUP_UPDATETIME) \
										values('%s', '%s', '%s','%d', '%d', '%d', '%s', '%s',from_unixtime('%ul'),from_unixtime('%ul'))";
	acl::string sql;

    //GenerateGroupId(m_pAccessPara->sGroupId,sAppId);
    char md5Buf[32];
    if(!pCreateInst->smsgid().empty()) 
	{
		CMd5::MD5_Calculate(pCreateInst->smsgid().c_str(), pCreateInst->smsgid().size(), md5Buf);
    }
    m_pAccessPara->sGroupId = sAppId + md5Buf;
    DbgLog("<<<<<<<<<<<groupId = <%s>>>>>>>>>>>>>>>>>>", m_pAccessPara->sGroupId.c_str());

	const db_groupstatus_t groupStatus = (TWOBARCODE_CREATION == pCreateInst->ncreatetype() || TWOBARCODE_CREATIONWITHHIDE == pCreateInst->ncreatetype()) ? DEPOSIT_GROUPSTATUS : NORMAL_GROUPSTATUS;
	sql.format(	sCreateSql,
			 	m_pAccessPara->sGroupId.c_str(),  //group id
				SqlFilter(pCreateInst->sname()).c_str(),		//specify group name
				SqlFilter(pCreateInst->sgrpavatar()).c_str(),	// specify group avatar
				1,								//default group count.
				pCreateInst->npermission(),		//specify group permission
				groupStatus,				//specify group status
				SqlFilter(pCreateInst->sremarks()).c_str(),	//specify group announcement.
				pCreateInst->suserid().c_str(),
				currentTime,
				currentTime);	//specify the creator userid. 

	if(CMysqlHelper::GetInstance()->Insert(m_hDB,sql.c_str()))
	{
		uint8_t bHide = (pCreateInst->ncreatetype()==NORMAL_CREATIONWITHHIDE||
					    pCreateInst->ncreatetype()==TWOBARCODE_CREATIONWITHHIDE) ? 
					    GROUP_HIDE_STYLE : GROUP_NONHIDE_STYLE;
		
		InsertGroupMember(pCreateInst->suserid(),m_pAccessPara->sGroupId,
							MASTER_MEMBER,bHide,CREATE_INSERT_MEMBER);
		
		string sCacheGroupId = PREFIX_GRPCACHE;
		sCacheGroupId += m_pAccessPara->sGroupId;	
		GroupInfo_t groupCache;

		groupCache.bPermit = pCreateInst->npermission();
		groupCache.bStatus = groupStatus;
		groupCache.nGroupNumber = 1;
		groupCache.sMasterId =pCreateInst->suserid().c_str();
		groupCache.sGroupId = sCacheGroupId.c_str();
		groupCache.createTime = currentTime;

        log("--------------redis insert group info begin-------------------");
		CRedisHelper::GetInstance()->InsertGroup(m_hashRedis,groupCache);
        log("--------------redis insert group info end-------------------");
		

		nResult = NON_ERR;
	}

	//DbgLog("************* End of insert group****************");
	return nResult;

}
ErrCode CDbTask::InsertGroupMember(string sUserId,string sGroupId,uint8_t bStatus,uint8_t bHide,db_memberinsertmode_t bMode)
{

	log("--------------insert group number begin-------------------");
	
	ErrCode nResult =	ERR_GROUP_NETWORKEXCEPTION;
	string sStatus = int2string(bStatus);
	const char* sMemberSql =	"insert into groupmember_t(GROUP_ID, MEMBER_ID,MEMBER_STATUS,MEMBER_REMARKS,hide_flag,MEMBER_CREATETIME,MEMBER_UPDATETIME) \
										values('%s', '%s', '%d', '%s','%d',now(),now())";
	acl::string sql;	
	string sUpdateSql = "on duplicate key update MEMBER_STATUS = '" + sStatus + "'";

	sql.format( sMemberSql,
			 	sGroupId.c_str(),
				sUserId.c_str(),
				bStatus,
				"",								//default member remarks
				bHide
			  );

	sql += sUpdateSql.c_str();
	if(CMysqlHelper::GetInstance()->Insert(m_hDB,sql.c_str()))
	{
		string sGroupKey = PREFIX_GRPMEMBERCACHE;
		sGroupKey += sGroupId;

        log("--------------redis insert group number begin-------------------");
		CRedisHelper::GetInstance()->InsertGroupMember(m_hashRedis,sGroupKey.c_str(),sUserId.c_str(),bStatus);
         log("--------------redis insert group number end-------------------");
		
		if(bMode==COMMON_INSERT_MEMBER)
		{	//update group count in normal inserting mode, don't  update 
			//the count any more in creating inserting mode.  
			UpdateGroupCount(sGroupId,1);
		}

		nResult = NON_ERR;
	}

	return nResult;
}

ErrCode CDbTask::InsertMultiGroupMember(string sGroupId,uint8_t bStatus)
{
	if(!m_pAccessPara->InviteeUserMap.size())
		return ERR_GROUP_INVITEEXCEPTION;
	
	ErrCode nResult =	ERR_GROUP_INVITEEXCEPTION;
	
	AccessUserMap_t::iterator it;
	RedisMemberMap_t memberMap;
	string sUserId;
	char sqlBatchBuf[SQL_BATCH_BUFSIZE] = {0}; 
	char rBuf[30] = {0};
	uint8_t bPermit;
	uint16_t nCount = 0;
	int nLen = 0;
	const char* sMember = "insert into groupmember_t(GROUP_ID, MEMBER_ID,MEMBER_STATUS,MEMBER_REMARKS,MEMBER_CREATETIME,MEMBER_UPDATETIME) values";
	string sMultiSql = sMember;
	string sUpdateSql = "on duplicate key update MEMBER_STATUS=values(MEMBER_STATUS);";
	
	snprintf(rBuf, sizeof(rBuf), "%d", bStatus);
	for (it = m_pAccessPara->InviteeUserMap.begin(); it != m_pAccessPara->InviteeUserMap.end(); it++) 
	{
		sUserId = it->first;
		bPermit = (uint8_t)it->second;
				
		if(!bPermit)
		{
			sprintf(sqlBatchBuf, "('%s','%s','%d','%s',now(),now()),",
				sGroupId.c_str(),sUserId.c_str(),bStatus,"");		// assemble multiple sql for batch processing.
			
			
			memberMap.insert(std::pair<acl::string,acl::string>(sUserId.c_str(),rBuf));
			
			sMultiSql += sqlBatchBuf;
			nCount++;
		}
	}
	
	if(nCount)
	{
		sprintf(sqlBatchBuf, "%s",sMultiSql.c_str());
		 nLen =  strlen(sqlBatchBuf);
		sqlBatchBuf[nLen-1] = ' ';
		sMultiSql = sqlBatchBuf;
		sMultiSql += sUpdateSql;

		
		if(CMysqlHelper::GetInstance()->Insert(m_hDB,sMultiSql.c_str()))
		{
			//Reserved for future restore redis info.
			//...
			//end of reserved. 
			string sGroupKey = PREFIX_GRPMEMBERCACHE;
			sGroupKey += sGroupId;
			CRedisHelper::GetInstance()->InsertGroupMember( m_hashRedis,sGroupKey.c_str(),memberMap);
			UpdateGroupCount(sGroupId,nCount);	
			
			//nResult = NON_ERR;
			nResult = ERR_GROUP_INVITESUCCESS;
		}	
	}

	return nResult;
}

ErrCode CDbTask::RemoveGroupMember(string sUserId,string sGroupId, bool bKick)
{
	ErrCode errCode  =	ERR_GROUP_NETWORKEXCEPTION;
	
	string sUpdateMemberSql = "update groupmember_t set MEMBER_STATUS = 0,MEMBER_UPDATETIME=now() where GROUP_ID = '"+
		sGroupId + "' and MEMBER_ID = '" + sUserId + "'";

	if(!bKick)  // 主动退群清理设置数据
		sUpdateMemberSql = "update groupmember_t set MEMBER_STATUS=0, hide_flag=0, sound_off=0, MEMBER_REMARKS='', MEMBER_UPDATETIME=now() where GROUP_ID = '" + sGroupId + "' and MEMBER_ID = '" + sUserId + "'";

	DbgLog("remove member %s from group %s",sUserId.c_str(),sGroupId.c_str());
	if(CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateMemberSql.c_str()))
	{
		string sGroupKey = PREFIX_GRPMEMBERCACHE;
		sGroupKey += sGroupId;
		
		CRedisHelper::GetInstance()->RemoveGroupMember(m_hashRedis, m_redis,sGroupKey.c_str(), sUserId.c_str());
		if(NON_ERR == UpdateGroupCount(sGroupId,1,GROUP_COUNT_DEC))
			WarnLog("update group count fail");
		errCode = NON_ERR;
	}

	return errCode;	
}

void CDbTask::ParseInviteeList(GroupInvite* pInstData)
{
	if(!pInstData)
		return; 
	string sGroupId = pInstData->sgroupid();
	string sUserId;
	uint8_t bMemberStatus = 0xff;
	uint8_t bGroupPermit = 0xff;
	ErrCode errCode;
	
	
	int i; 

	for(i = 0; i < pInstData->sinviteeids_size(); i++)
	{
		sUserId = pInstData->sinviteeids(i);
		//if(IsMember(sUserId,sGroupId,bMemberStatus)==ERR_GROUP_MEMBERNONEXIST)
		errCode = IsMember(sUserId,sGroupId,bMemberStatus);
		if(((errCode == ERR_GROUP_MEMBERNONEXIST)||((errCode == ERR_GROUP_MEMBEREXIST)
			&& (bMemberStatus==QUIT_MEMBER)))&&VerifyGroupUserPermission(sUserId,bGroupPermit)==NON_ERR)
		{
			m_pAccessPara->InviteeUserMap[sUserId] = bGroupPermit;  //default set to no permission.
		}
	}
	DbgLog("Invitee list : %d",pInstData->sinviteeids_size());
}


void CDbTask::ParseCreateInvitee(GroupCreate* pInstData)
{
	if(!pInstData)
		return; 
	string sGroupId = m_pAccessPara->sGroupId;
	string sUserId;
	uint8_t bMemberStatus = 0xff;
	uint8_t bGroupPermit = 0xff;
	ErrCode errCode;
	int i;
	
	for(i = 0; i < pInstData->sinviteuserids_size(); i++)
	{
		sUserId = pInstData->sinviteuserids(i);

		errCode = IsMember(sUserId,sGroupId,bMemberStatus);
		//if(IsMember(sUserId,sGroupId,bMemberStatus)==ERR_GROUP_MEMBERNONEXIST)

		//if((errCode == ERR_GROUP_MEMBERNONEXIST) || ((errCode == ERR_GROUP_MEMBEREXIST)
		//	&& (bMemberStatus==QUIT_MEMBER)))
		//{
		//	m_pAccessPara->InviteeUserMap[sUserId] = 0;  //default set to no permission.
		//}
		if(((errCode == ERR_GROUP_MEMBERNONEXIST)||((errCode == ERR_GROUP_MEMBEREXIST)
			&& (bMemberStatus==QUIT_MEMBER)))&&VerifyGroupUserPermission(sUserId,bGroupPermit)==NON_ERR)
		{
			m_pAccessPara->InviteeUserMap[sUserId] = bGroupPermit;  //default set to no permission.
		}		
	}
	DbgLog("Create Invitee list : %d",pInstData->sinviteuserids_size());

}

ErrCode CDbTask::CreateInviteGroup(GroupCreate* pInstData)
{
	if(m_pAccessPara->sGroupId.empty() || !pInstData)
		return ERR_GROUP_INVITEEXCEPTION;

	ErrCode nResult = ERR_GROUP_INVITEEXCEPTION;
	
	ParseCreateInvitee(pInstData);
	nResult = InsertMultiGroupMember(m_pAccessPara->sGroupId, INVITE_MEMBER);	

	return nResult;
}

ErrCode  CDbTask::ApplyGroup(void)
{
	GroupApply* pUserData = (GroupApply*)m_pAccessPara->pUserData;
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;
	string sGroupId = pUserData->sgroupid();
	string sUserId = pUserData->suserid();
	string sCacheUser = PREFIX_CMCACHE;
	uint8_t bMemberStatus = 0xff;
	UserCache_t userCache;
		

	if((errCode = VerifyGroupJoiningAuth(sGroupId, 0, MAX_GROUPMEMBER, 1, pUserData->applytype())) != NON_ERR)
	{
		WarnLog("join forbiddened 0x%x when processing user %s apply group %s!",
					errCode ,sUserId.c_str(),sGroupId.c_str());
		return errCode;
	}
	
	sCacheUser += sUserId;
	if(CRedisHelper::GetInstance()->GetUserInfo(m_hashRedis,sCacheUser.c_str(),userCache))
	{
		//reserved for checking user authority.
		//...
		//end of reserved . 
		errCode = IsMember(sUserId,sGroupId,bMemberStatus);
		if((errCode == ERR_GROUP_MEMBERNONEXIST) || ((errCode == ERR_GROUP_MEMBEREXIST) && (bMemberStatus==QUIT_MEMBER)))		
		{
			GroupInfo_t info;
			info.sGroupId = sGroupId.c_str();

			if (GetGroupInfo(info))
			{
				m_pAccessPara->sMasterId = info.sMasterId;
				
				errCode = _GroupPermissionCode[info.bPermit];
				
				DbgLog("Group %s permission %d,state %d", sGroupId.c_str(), info.bPermit, info.bStatus);
				if (NORMAL_GROUPSTATUS == info.bStatus || DEPOSIT_GROUPSTATUS == info.bStatus)
				{
					if (0 == info.bPermit)	//不需要允许
					{
						errCode = InsertGroupMember(sUserId, sGroupId, APPLY_MEMBER);
					}
					if (NON_ERR == errCode && DEPOSIT_GROUPSTATUS == info.bStatus)
					{
						errCode = UpdateGroupStatus(sGroupId, NORMAL_GROUPSTATUS);
						string sGroupKey = PREFIX_GRPCACHE;
						sGroupKey += sGroupId;
						CRedisHelper::GetInstance()->UpdateGroupStatus(m_hashRedis, m_redis, sGroupKey.c_str(), NORMAL_GROUPSTATUS);
						m_pAccessPara->grpNewState = NORMAL_GROUPSTATUS;
					}

				}
			}
		}
		else
		{
			return ERR_GROUP_MEMBEREXIST;
		}
	}
	else
	{
		ErrLog("Group apply is forbidden , maybe the applied user %s is abnormal",pUserData->suserid().c_str());
		errCode = ERR_GROUP_FORBIDDEN;
	}

	return errCode;
}

ErrCode CDbTask::InviteGroup(void)
{
	GroupInvite* pUserData = (GroupInvite*)m_pAccessPara->pUserData;
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;
	string sGroupId = pUserData->sgroupid();
	string sUserId = pUserData->sinviterid();
	UserCache_t userCache;
	string sCacheUser = PREFIX_CMCACHE;
		
	if((errCode = VerifyGroupJoiningAuth(sGroupId,0,MAX_GROUPMEMBER,pUserData->sinviteeids_size()))!=NON_ERR)
	{
		WarnLog("Encounter err 0x%x when processing user %s invite group %s!",
					errCode ,sUserId.c_str(),sGroupId.c_str());
		return errCode;
	}


	sCacheUser += sUserId;
		
	if(CRedisHelper::GetInstance()->GetUserInfo(m_hashRedis,sCacheUser.c_str(),userCache))
	{
		//reserved for checking user authority.
		//...
		//end of reserved . 
		ParseInviteeList(pUserData);       // Check whether or not the invitee user is group member and it's permission attr.
		errCode = InsertMultiGroupMember(sGroupId, INVITE_MEMBER);
	}
	else
	{
		ErrLog("Group invite is forbidden , maybe the inviter user %s is abnormal",sUserId.c_str());
		errCode = ERR_GROUP_FORBIDDEN;
	}
	return errCode;
}

ErrCode  CDbTask::QuitGroup(void)
{
	void* pUserData = m_pAccessPara->pUserData;
	string sGroupId = ((GroupQuit*)pUserData)->sgroupid();
	string sUserId = ((GroupQuit*)pUserData)->suserid();
	uint8_t bMemberStatus =  0xff;
						
	log("--------------quit group begin-------------------");
	ErrCode  errCode = IsMember(sUserId,sGroupId,bMemberStatus);
	if(ERR_GROUP_MEMBEREXIST != errCode)	// The user is group member
	{
		if(ERR_GROUP_MEMBERNONEXIST == errCode) 
			return ERR_GROUP_MEMBERNONEXIST;
		return ERR_GROUP_FORBIDDEN;
	}

	GroupInfo_t grpInfo;
	grpInfo.sGroupId = sGroupId.c_str();
	if(!GetGroupInfo(grpInfo))
	{
		errCode = ERR_GROUP_NETWORKEXCEPTION;
		return errCode;
	}
	else if(grpInfo.bStatus != NORMAL_GROUPSTATUS )
	{
		errCode = ERR_GROUP_FORBIDDEN;
		return errCode;
	}
	else
	{
		if(grpInfo.sMasterId.compare(sUserId.c_str()) == 0 && grpInfo.nGroupNumber > 1)
		{
			errCode = ERR_GROUP_MASTER_QUIT;
			return errCode;
		}
	}
	errCode = RemoveGroupMember(sUserId,sGroupId, false);
	return errCode;
}

ErrCode  CDbTask::KickoutGroup(void)
{
	void* pUserData = m_pAccessPara->pUserData;
	string sGroupId = ((GroupKickOut*)pUserData)->sgroupid();
	uint8_t bMemberStatus =  0xff;
	
	
	ErrCode  errCode = IsMember(((GroupKickOut*)pUserData)->skickid(),sGroupId,bMemberStatus);
	if(ERR_GROUP_MEMBEREXIST == errCode)	// The kick user is group member
	{
		if(IsMaster(((GroupKickOut*)pUserData)->suserid(),sGroupId)==ERR_GROUP_MASTER)
			errCode = RemoveGroupMember(((GroupKickOut*)pUserData)->skickid(),sGroupId);
		else
			errCode = ERR_GROUP_FORBIDDEN; 

		return errCode;
	}

	if(ERR_GROUP_MEMBERNONEXIST == errCode) return ERR_GROUP_MEMBERNONEXIST;
	
	return ERR_GROUP_FORBIDDEN; //forbben to execute current operation. 
}

ErrCode CDbTask::ModifyGroupAnnouncement(string sUserId,string sGroupId,string sContent)
{
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;

	errCode = IsMaster(sUserId,sGroupId);
	if(errCode==ERR_GROUP_MASTER)
	{
		string sUpdateAnnounceSql = "update group_t set  GROUP_UPDATETIME=now(),GROUP_ANNOUNCEMENT = '" + 
									sContent + "'where GROUP_ID = '" + sGroupId + "'";

		errCode = CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateAnnounceSql.c_str()) ? NON_ERR : ERR_GROUP_NETWORKEXCEPTION;
		return errCode;
	}
	
	return ERR_GROUP_FORBIDDEN; //forbben to execute current operation. 
}

ErrCode CDbTask::ModifyGroupName(string sUserId,string sGroupId,string sContent)
{
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;

	errCode = IsMaster(sUserId,sGroupId);
	if(errCode==ERR_GROUP_MASTER)
	{
		string sUpdateNameSql = "update group_t set  GROUP_UPDATETIME=now(),GROUP_NAME = '" + 
								sContent + "'where GROUP_ID = '" + sGroupId + "'";

		errCode = CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateNameSql.c_str()) ? NON_ERR : ERR_GROUP_NETWORKEXCEPTION;
		return errCode;
	}
	
	return ERR_GROUP_FORBIDDEN; //forbben to execute current operation. 
}


ErrCode CDbTask::ModifyGroupMaster(string sUserId,string sGroupId,string sInvolvedId)
{
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;

	errCode = IsMaster(sUserId,sGroupId) ;
	if(errCode==ERR_GROUP_MASTER)
	{
		// 判断成员是否存在
		uint8_t bMemberStatus =  0xff;
		errCode = IsMember(sUserId,sGroupId, bMemberStatus);
		if(errCode != ERR_GROUP_MEMBEREXIST || bMemberStatus == QUIT_MEMBER)
		{
			return ERR_GROUP_MEMBERNONEXIST;
		}
		
		string sUpdateMasterSql = "update group_t set  GROUP_UPDATETIME=now(),GROUP_MASTERID = '" + 
									sInvolvedId + "'where GROUP_ID = '" + sGroupId + "'";

		errCode = CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateMasterSql.c_str()) ? NON_ERR : ERR_GROUP_NETWORKEXCEPTION;
		if(NON_ERR == errCode)
		{
			string sUpdateMemberSql = "update groupmember_t set MEMBER_UPDATETIME=now(),MEMBER_STATUS = 1 where GROUP_ID = '"+
									sGroupId + "' and MEMBER_ID = '" + sUserId + "'";
			CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateMemberSql.c_str());
			
			sUpdateMemberSql = "update groupmember_t set MEMBER_UPDATETIME=now(),MEMBER_STATUS = 3 where GROUP_ID = '"+
									sGroupId + "' and MEMBER_ID = '" + sInvolvedId + "'";
			CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateMemberSql.c_str());

			string sGroupKey = PREFIX_GRPMEMBERCACHE;
			sGroupKey += sGroupId;
			CRedisHelper::GetInstance()->InsertGroupMember(m_hashRedis,sGroupKey.c_str(),sUserId.c_str(),1);
			CRedisHelper::GetInstance()->InsertGroupMember(m_hashRedis,sGroupKey.c_str(),sInvolvedId.c_str(),3);

		
			sGroupKey = PREFIX_GRPCACHE;
			sGroupKey += sGroupId;
			//DbgLog("Modify group master redis : %s,%s",sGroupKey.c_str(),sInvolvedId.c_str());
			CRedisHelper::GetInstance()->UpdateGroupMaster(m_hashRedis, sGroupKey.c_str(), sInvolvedId.c_str());

		}
		return errCode;
	}
	
	return ERR_GROUP_FORBIDDEN; //forbben to execute current operation. 
}


ErrCode CDbTask::ModifyGroupAvatar(string sUserId,string sGroupId,string sContent)
{
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;
	
	errCode = IsMaster(sUserId,sGroupId);
	if(errCode==ERR_GROUP_MASTER)
	{
		string sUpdateAvatarSql = "update group_t set  GROUP_UPDATETIME=now(),GROUP_AVATAR = '" + 
								sContent + "'where GROUP_ID = '" + sGroupId + "'";
	
		errCode = CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateAvatarSql.c_str()) ? 
					NON_ERR : ERR_GROUP_NETWORKEXCEPTION;
		return errCode;
	}
		
	return ERR_GROUP_FORBIDDEN; //forbben to execute current operation. 
}

ErrCode CDbTask::ModifyGroupNickName(string sUserId,string sGroupId,string sContent)
{
	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;
	uint8_t bMemberStatus =  0xff;

	errCode = IsMember(sUserId,sGroupId,bMemberStatus);
	if(errCode==ERR_GROUP_MEMBEREXIST)
	{
		string sUpdateNickNameSql = "update groupmember_t set  MEMBER_UPDATETIME=now(),MEMBER_REMARKS = '" + 
			sContent + "'where GROUP_ID = '"+ sGroupId + "' and MEMBER_ID = '" + sUserId + "'";

		errCode = CMysqlHelper::GetInstance()->Update(m_hDB, sUpdateNickNameSql.c_str()) ? 
					NON_ERR : ERR_GROUP_NETWORKEXCEPTION;
		return errCode;
	}
	
	return ERR_GROUP_FORBIDDEN; //forbben to execute current operation. 
}
/////////////////////////////////////////////////////////////////////////////////////////
// Task of creation procedure
////////////////////////////////////////////////////////////////////////////////////////

void CDbTask::CreateGroupProc(void)
{

	log("--------------create group begin-------------------");

	GroupCreate* pUserData = (GroupCreate*)m_pAccessPara->pUserData;
	ErrCode errCode = im::ERR_GROUP_NETWORKEXCEPTION;
	UserCache_t userCache;
	
	string sCacheUserId = PREFIX_CMCACHE;
	sCacheUserId += pUserData->suserid();

	do{

		//check groupNumber
		if(pUserData->sinviteuserids_size() > MAX_GROUPMEMBER)
		{
			errCode = ERR_GROUP_OVERJOIN;
			ErrLog("Encounter err 0x%x inviteNum %d when user %s requesting to create a new group!",
						errCode ,pUserData->sinviteuserids_size(),pUserData->suserid().c_str());
			break;
		}

		log("--------------get master user info begin-------------------");
		if(CRedisHelper::GetInstance()->GetUserInfo(m_hashRedis,sCacheUserId.c_str(),userCache))
		{
			log("--------------get master user info end-------------------");
			//if(userCache.nGroupNumber > userCache.nGroupLimit)
			if((errCode = VerifyGroupCreatingAuth(pUserData->suserid(),userCache.bRole,
				userCache.nGroupLimit)) != NON_ERR)
			{
				//WarnLog("User % group creation is out of limit!",pUserData->suserid().c_str());
				WarnLog("Encounter err 0x%x when user %s requesting to create a new group!",
						errCode ,pUserData->suserid().c_str());
			}
			else
			{
				log("--------------insert group info to database begin-------------------");
				errCode = InsertGroup(pUserData,userCache.sAppId.c_str());
				if(errCode==NON_ERR && (pUserData->ncreatetype()==NORMAL_CREATION ||
					pUserData->ncreatetype()==NORMAL_CREATIONWITHHIDE))
				{
					DbgLog("Group %s is created successfully by user %s",
						m_pAccessPara->sGroupId.c_str(),pUserData->suserid().c_str());
					errCode = CreateInviteGroup(pUserData);
				}
				
				log("--------------insert group info to database begin-------------------");
				
			}
		}
		else
		{
			ErrLog("Group creation is forbidden , maybe the user %s maybe is abnormal",pUserData->suserid().c_str());
			errCode = ERR_GROUP_FORBIDDEN;
		}
	}while(0);
	
	(*m_accessCb)(m_pPacketProcess,m_pAccessPara,errCode);	
}
/////////////////////////////////////////////////////////////////////////////////////////
// Task of joining procedure
////////////////////////////////////////////////////////////////////////////////////////

void CDbTask::JoinGroupProc(void)
{
	ErrCode errCode = (m_pAccessPara->bMode == APPLY_JOIN_MODE) ? ApplyGroup() : InviteGroup();
	(*m_accessCb)(m_pPacketProcess,m_pAccessPara,errCode);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Task of directly joining procedure
////////////////////////////////////////////////////////////////////////////////////////

void CDbTask::JoinDirectlyGroupProc(void)
{
	void* pUserData = m_pAccessPara->pUserData;
	uint8_t bMode = ((GroupPermit*)pUserData)->rsptype();
	string sGroupId = ((GroupPermit*)pUserData)->sgrpid();
	string sUserId = (!bMode) ? ((GroupPermit*)pUserData)->stoid() : 
					((GroupPermit*)pUserData)->sfromid();

	ErrCode errCode = NON_ERR; 
	GroupInfo_t grpInfo;
	grpInfo.sGroupId = sGroupId.c_str();
	if(GetGroupInfo(grpInfo) && grpInfo.bStatus == NORMAL_GROUPSTATUS)
	{
		// 判断成员是否存在
		uint8_t bMemberStatus =  0xff;
		errCode = IsMember(sUserId,sGroupId, bMemberStatus);
		if(errCode == im::ERR_GROUP_MEMBERNONEXIST || bMemberStatus == QUIT_MEMBER)
		{
			uint8_t bStatus = (!bMode) ? APPLY_MEMBER : INVITE_MEMBER;							//default member status
			errCode = InsertGroupMember(sUserId, sGroupId,bStatus);
			log("---------- JoinDirectlyGroupProc errcode %d ---------", errCode);
		}
		else
		{
			errCode = ERR_GROUP_MEMBEREXIST;		// 已是群组成员
		}
	}
	else
	{
		errCode = ERR_GROUP_DISMISSED;		// 群组不存在或已解散
	}
	
	(*m_accessCb)(m_pPacketProcess,m_pAccessPara,errCode);	
}
/////////////////////////////////////////////////////////////////////////////////////////
// Task of removing procedure
////////////////////////////////////////////////////////////////////////////////////////
void CDbTask::RemoveGroupProc(void)
{
	ErrCode errCode  = (m_pAccessPara->bMode == QUIT_REMOVE_MODE) ? QuitGroup() : KickoutGroup();	
	(*m_accessCb)(m_pPacketProcess,m_pAccessPara,errCode);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Task of modification procedure
////////////////////////////////////////////////////////////////////////////////////////

void CDbTask::ModifyGroupProc(void)
{
	GroupModify* pUserData = (GroupModify*)m_pAccessPara->pUserData;

	ErrCode errCode = ERR_GROUP_NETWORKEXCEPTION;
	NotifyType nType = pUserData->nmodifytype();
	string sUserId  = pUserData->suserid();
	string sGroupId = pUserData->sgroupid();
	string sContent = pUserData->scontent();
	string sInvolvedId = pUserData->sinvolvedid();

	switch(nType)
	{
		case NOTIFY_TYPE_GRP_ANNOUNCEMENT:
			errCode = ModifyGroupAnnouncement(sUserId,sGroupId,sContent);
			break;
		case NOTIFY_TYPE_GRPNAME_MODIFIED:
			errCode = ModifyGroupName(sUserId,sGroupId,sContent);
			break;
		case NOTIFY_TYPE_GRPMASTER_CHANGED:
			errCode = ModifyGroupMaster(sUserId,sGroupId, sInvolvedId);
			break;
		case NOTIFY_TYPE_GRPPHOTO_MODIFIED:
			errCode = ModifyGroupAvatar(sUserId,sGroupId,sContent);
			break;		
		case NOTIFY_TYPE_CLINICKNAME_MODITIED:
			errCode = ModifyGroupNickName(sUserId,sGroupId,sContent);
			break;
		default:
			break;

	}

	(*m_accessCb)(m_pPacketProcess,m_pAccessPara,errCode);
}
/////////////////////////////////////////////////////////////////////////////////////////
// Task run procedure
////////////////////////////////////////////////////////////////////////////////////////
void CDbTask::run()
{
	//DbgLog("Now try to excute task: %d",m_accessTask);
	
	if(!m_pAccessRedisCluster || !m_pAccessSqlPool ||
		!m_pPacketProcess || !m_accessCb || !m_pAccessPara)
	{
		ErrLog("Fail to excute task because some resource is null");
		return;
	}

	switch(m_accessTask)
	{
		case CREATE_GROUP_TASK:
			CreateGroupProc();
			break;
		case JOIN_GROUP_TASK:
			JoinGroupProc();
			break;
		case DIRECTLYJOIN_GROUP_TASK:
			JoinDirectlyGroupProc();
			break;
		case REMOVE_GROUP_TASK:
			RemoveGroupProc();
			break;
		case MODIFY_GROUP_TASK:
			ModifyGroupProc();
		default:
			break;
	}
}
		

