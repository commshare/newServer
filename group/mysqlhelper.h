#ifndef __MYSQLHELPER_H__
#define __MYSQLHELPER_H__

#include <string>
#include "acl_cpp/lib_acl.hpp"
#include "configfilereader.h"
#include "singleton.h"
#include "redishelper.h"
//#include "threadpool.h"
//#include "task.h"
//#include "packet.h"
//#include "im.group.pb.h"


using namespace std;

#define MEMBERSTATUS_POS 3
#define USERPERMISSION_POS 32

typedef enum _MYSQL_ACCESS_TASK          //Associated identification
{
	MYSQL_EXCEPTION_ACCESS,
	MYSQL_INSERT_ACCESS,
	MYSQL_DEL_ACCESS,		
	MYSQL_UPDATE_ACCESS,
	MYSQL_SELECT_ACCESS
} MYSQL_ACCESS_TASK;

	
/*typedef struct _CreateUserData 
{
	string sUserId;
	string sGroupId;
	string sGroupName;
	string sMsgId;
	string sAnnouncement;
	uint8_t bPermission;
	uint8_t bStatus;
	uint8_t bCreateType;
	UidCode_t sessionId;
} CreateUserData_t;
*/
typedef enum _GROUP_ROW
{
	GROUPID_ROW,
	GROUPNAME_ROW,
	GROUPAVATAR_ROW,
	GROUPCOUNT_ROW,
	GROUPPERMISSION_ROW,
	GROUPSTATUS_ROW,
	GROUPANNOUNCEMENT_ROW,
	GROUPMASTERID_ROW
} GROUP_FIELD;


class CMysqlHelper : public Singleton<CMysqlHelper>
{
public:
	CMysqlHelper();
	~CMysqlHelper();

	bool Initialize(CConfigFileReader* pConfigReader);	
	acl::db_pool* GetPool() {return m_pMysqlPool;}
	bool GetUserPermission(acl::db_handle* hDB,const char *sSql,uint8_t &bPermit);
	bool GetMemberStatus(acl::db_handle* hDB,const char *sSql,uint8_t &bStatus);
	bool GetGroupNumber(acl::db_handle* hDB,const char *sSql,uint16_t &nNumber);
	//bool GetGroupMaster(acl::db_handle* hDB,const char *sSql,string &sMasterId,uint8_t &bPermit);
	bool GetGroupInfo(acl::db_handle* hDB, const string& sGroupId, GroupInfo_t &info);
	bool Insert(acl::db_handle* hDB,const char*  sSql);
	bool Update(acl::db_handle* hDB,const char*  sSql);
protected:
	virtual bool SetMysqlHelper(void);	  	//Load mysql config parameter from Config file. 
private:
	CConfigFileReader* m_pConfigReader;	
	acl::db_pool* m_pMysqlPool;
	string m_sMysqlUrl;			//mysql host , support in single or cluster mode
	string m_sDbName;
	string m_sDbUser;
	string m_sDbPass;
};


#endif // __MYSQLHELPER_H__

