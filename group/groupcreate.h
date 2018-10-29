#ifndef __GROUPCREATE_H__
#define __GROUPCREATE_H__
#include "configfilereader.h"
#include "packet.h"
#include "dbhelper.h"
#include "im.pub.pb.h"
#include "im.mes.pb.h"
#include "im.group.pb.h"
#include "groupnotify.h"




using namespace std;
using namespace im;
//using namespace imsvr;


class CGroupCreate : public CPacket,public CDbHelper
{
public:
	CGroupCreate(CConfigFileReader* pConfigReader,int nNumOfInst);
	~CGroupCreate();

	bool Initialize(CConfigFileReader* pConfigReader);	

	bool OnCreate(std::shared_ptr<CImPdu> pPdu); 
	void SendNotify(AccessUserPara_t* pAccessUserPara);
	void InviteeUserJoins(AccessUserPara_t* pAccessUserPara);	
	static void OnCreateStartup(CPacket* pAccessPack,AccessUserPara_t* pAccessUserPara,ErrCode errCode); //
	
protected:
	virtual bool RegistPacketExecutor(void);
	bool CreateProc(GroupCreate* pCreateInstData,UidCode_t sessionId);
	bool CreateStartup(GroupCreate* pCreateInstData,UidCode_t sessionId);
	void CreateRsp(GroupCreate* pCreateInst,string sGroupId,UidCode_t sessionId,ErrCode bCode,uint64_t ackTime=0);
	int JoinReq(string sFromId,string sToId,string sGroupId,string sMsgId,uint8_t bMode, const string& strExtend);
private:
	CConfigFileReader* m_pConfigReader;
	int				   m_nNumberOfInst;
	//CCache* m_pCache;		//Redis cache pointer . 
};


#endif
