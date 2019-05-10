#include "commandlist.h"
#include "public_define.h"
#include "Common.h"
#include "ConfigFileReader.h"
//#include "IM.Monitor.pb.h"
//#include "IM.BaseDefine.pb.h"
//#include "IM.DBA.pb.h"
//#include "IM.Message.pb.h"
#include "im.cm.pb.h"
#include "im.mes.pb.h"
#include "im.group.pb.h"
#include "im.sig.pb.h"
#include "userInfoMgr.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>  
uint64_t getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;	//将秒和微秒转换成毫秒
}


//using namespace IM::BaseDefine;
//using namespace IM::Message;
using namespace im;


extern void send_to_all_server(CImPdu* pPdu);

typedef void (*IMCmdFunPtr)(void* );
typedef hash_map<uint32_t, IMCmdFunPtr> CmdMap_t;
CmdMap_t g_cmdMap;

string gSessionId;


static void _AssocSvrRegist(void*)
{
	CConfigFileReader config_file(GetConfigFileName().c_str());

	printf("_AssocSvrRegist called\r\n");
	char* SvrRegIp = config_file.GetConfigName("link_ip");	
	char* SvrRegPort = config_file.GetConfigName("link_port");	
	string 	 sIp = (NULL == SvrRegIp ? "192.168.199.92": string(SvrRegIp));
	uint16_t nPort =  (NULL == SvrRegPort ? 1902: atoi(SvrRegPort));
	printf("nPort = %d\r\n", nPort);
	SYSAssocSvrRegist regist;
	regist.set_ip(sIp);
	regist.set_port(nPort);

	string m_sSessionId="";
	CImPdu pdu;
	pdu.SetPBMsg(&regist);
	pdu.SetCommandId(SYSTEM_ASSOCSVR_REGIST);
	pdu.SetSessionId("");
	send_to_all_server(&pdu);
}

static void _LoginReq(void*)
{
//    CConfigFileReader config_file(GetConfigFileName().c_str());
	CConfigFileReader config_file(GetConfigFileName().c_str());

    //char* sessionId = config_file.GetConfigName("sessionid");

    char* str_userId = config_file.GetConfigName("userId");
	char* str_usertoken = config_file.GetConfigName("userToken");
    //char* str_devicetype = config_file.GetConfigName("deviceType");
	//char* str_deviceversion = config_file.GetConfigName("deviceVersion");
	char* str_devicetoken = config_file.GetConfigName("deviceToken");
    
    CMLogin loginReqMsg;
    loginReqMsg.set_suserid(str_userId);
    loginReqMsg.set_slogintoken(str_usertoken);
	//loginReqMsg.set_ndevicetype(atoi(str_devicetype));
   	//loginReqMsg.set_sdeviceversion(str_deviceversion);
    loginReqMsg.set_sdevicetoken(str_devicetoken);

    CImPdu pdu;
	printf("login user=%s, token=%s",str_userId,str_usertoken);
    pdu.SetPBMsg(&loginReqMsg);
    //pdu.SetSessionId(atoll(sessionId)+99999900);
    pdu.SetCommandId(CM_LOGIN);
    send_to_all_server(&pdu);
}

static void _LoginOutReq(void*)
{
    CConfigFileReader config_file(GetConfigFileName().c_str());

    //char* sessionId = config_file.GetConfigName("sessionid");

    char* str_userId = config_file.GetConfigName("userId");
    char* str_devicetype = config_file.GetConfigName("deviceType");
    
    CMLogout loginOutMsg;
    loginOutMsg.set_suserid(str_userId);
    loginOutMsg.set_ndevicetype(atoi(str_devicetype));

    CImPdu pdu;
    pdu.SetPBMsg(&loginOutMsg);
    pdu.SetSessionId(gSessionId);
    pdu.SetCommandId(CM_LOGOUT);
    send_to_all_server(&pdu);
}

static void _AddBuddy(void*)
{
	CConfigFileReader config_file(GetConfigFileName().c_str());

//	char* sessionId = config_file.GetConfigName("sessionid");
	

	   char* fromuserId = config_file.GetConfigName("userId");
//	   char* userNickname = config_file.GetConfigName("userNickname");
	
	   char* touserId = config_file.GetConfigName("peerUid");
//	   char* peerNickname = config_file.GetConfigName("peerNickname");
//	   char* msg = config_file.GetConfigName("msg");

		  
	  MESAddFriend addFriendMsg;
	  addFriendMsg.set_sfromid(fromuserId);
	  addFriendMsg.set_stoid(touserId);
	  addFriendMsg.set_smsgid("add me");
	  addFriendMsg.set_sdesc("my friend");
	  static int count = 0;
	  if (++count % 2)
	  {
		  addFriendMsg.set_smemoname("nickName");
      }
	  
	
	  CImPdu pdu;
	  pdu.SetPBMsg(&addFriendMsg);
	  pdu.SetCommandId(MES_ADDFRIEND);
	  pdu.SetSessionId(gSessionId);
	  send_to_all_server(&pdu);
	  printf("**	  addFriendReq called,from %s - %s\r\n", fromuserId, touserId);
		

}


static void _SponsorCal(void*)
{
	CConfigFileReader config_file(GetConfigFileName().c_str());

//	char* sessionId = config_file.GetConfigName("sessionid");
	

	   char* fromuserId = config_file.GetConfigName("userId");
//	   char* userNickname = config_file.GetConfigName("userNickname");
	
	   char* touserId = config_file.GetConfigName("peerUid");
//	   char* peerNickname = config_file.GetConfigName("peerNickname");
//	   char* msg = config_file.GetConfigName("msg");

		  
	  SIGSponsorCall SponsorCall;
	  SponsorCall.set_sfromid(fromuserId);
	  SponsorCall.set_sinviteid(touserId);

	  string msgId = getuuid();
	  transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);	
	  SponsorCall.set_smsgid(msgId);
	
	  CImPdu pdu;
	  pdu.SetPBMsg(&SponsorCall);
	  pdu.SetCommandId(SIG_SPONSORP2PCALL);
	  pdu.SetSessionId(gSessionId);
	  send_to_all_server(&pdu);
	  printf("**	  SponsorCall called,from %s - %s\r\n", fromuserId, touserId);
}

static void _SponsorCalAns(void*)
{
	CConfigFileReader config_file(GetConfigFileName().c_str());

//	char* sessionId = config_file.GetConfigName("sessionid");
	

	   char* fromuserId = config_file.GetConfigName("userId");
//	   char* userNickname = config_file.GetConfigName("userNickname");
	
	   char* touserId = config_file.GetConfigName("peerUid");
//	   char* peerNickname = config_file.GetConfigName("peerNickname");
//	   char* msg = config_file.GetConfigName("msg");

		  
	  SIGSponsorCallAns req;
	  req.set_sfromid(fromuserId);
	  req.set_stoid(touserId);

	  string msgId = getuuid();
	  transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);	
	  req.set_smsgid(msgId);
	
	  CImPdu pdu;
	  pdu.SetPBMsg(&req);
	  pdu.SetCommandId(SIG_SPONSORP2PCALL_ANS);
	  pdu.SetSessionId(gSessionId);
	  send_to_all_server(&pdu);
	  printf("**	  SponsorCallAns called,from %s - %s\r\n", fromuserId, touserId);
}

static void _exchangeNatInfo(void*)
{
	CConfigFileReader config_file(GetConfigFileName().c_str());

//	char* sessionId = config_file.GetConfigName("sessionid");
	

	   char* fromuserId = config_file.GetConfigName("userId");
//	   char* userNickname = config_file.GetConfigName("userNickname");
	
	   char* touserId = config_file.GetConfigName("peerUid");
//	   char* peerNickname = config_file.GetConfigName("peerNickname");
//	   char* msg = config_file.GetConfigName("msg");

		  
	  SIGP2PCallExchangeNatInfo req;
	  req.set_sfromid(fromuserId);
	  req.set_stoid(touserId);
	  req.set_scontent("change nat info");

	  string msgId = getuuid();
	  transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);	
	  req.set_smsgid(msgId);
	
	  CImPdu pdu;
	  pdu.SetPBMsg(&req);
	  pdu.SetCommandId(SIG_P2PCALL_EXCHANGE_NATINFO);
	  pdu.SetSessionId(gSessionId);
	  send_to_all_server(&pdu);
	  printf("**	  SponsorCallAns called,from %s - %s\r\n", fromuserId, touserId);
}



static void _DelBuddy(void*)
{
	CConfigFileReader config_file(GetConfigFileName().c_str());

//	char* sessionId = config_file.GetConfigName("sessionid");
	

//	   char* fromuserId = config_file.GetConfigName("userId");
//	   char* userNickname = config_file.GetConfigName("userNickname");
	
//	   char* touserId = config_file.GetConfigName("peerUid");
//	   char* peerNickname = config_file.GetConfigName("peerNickname");
//	   char* msg = config_file.GetConfigName("msg");
}


static void _AddBuddyAns(void*)
{
	CConfigFileReader config_file(GetConfigFileName().c_str());

//	char* sessionId = config_file.GetConfigName("sessionid");
	

	char* fromuserId = config_file.GetConfigName("userId");
//	char* userNickname = config_file.GetConfigName("userNickname");
	
	char* touserId = config_file.GetConfigName("peerUid");
//	char* peerNickname = config_file.GetConfigName("peerNickname");
//	char* msg = config_file.GetConfigName("msg");

		  
	MESAddFriendAns addFriendAns;
	addFriendAns.set_sfromid(fromuserId);
	addFriendAns.set_stoid(touserId);
	addFriendAns.set_smsgid("add me");
	addFriendAns.set_sans("my friend");
	addFriendAns.set_errcode(NON_ERR);
	addFriendAns.set_smemoname("rose");
	addFriendAns.set_packetid(1);
		
	CImPdu pdu;
	pdu.SetPBMsg(&addFriendAns);
	pdu.SetCommandId(MES_ADDFRIEND_ANS);
	pdu.SetSessionId(gSessionId);
	send_to_all_server(&pdu);
	printf("**	  addFriendAns called,from %s - %s\r\n", fromuserId, touserId);

}

static void _GroupCreate(void*)
{
	CConfigFileReader config_file("client.conf");
    char* str_userId = config_file.GetConfigName("userId");
	char* str_permission = config_file.GetConfigName("permission");
	char* str_createtype = config_file.GetConfigName("createtype");
    
    GroupCreate grpCreateMsg;
    grpCreateMsg.set_suserid(str_userId);
    grpCreateMsg.set_sname("group create testing");
	grpCreateMsg.set_sremarks("group announcement");
	grpCreateMsg.set_smsgid("smsgid001");
	grpCreateMsg.set_npermission(atoi(str_permission));
	grpCreateMsg.set_ncreatetype(atoi(str_createtype));
	//grpCreateMsg.add_sinviteuserids("10024");
	//grpCreateMsg.add_sinviteuserids("10025");


	int i = 0;

	char msgId[256] = {0};
	while(i<1)
	{
		std::shared_ptr<UserInfo> pUser1 = UserInfoMgr::getRandomUserInfo();
		std::shared_ptr<UserInfo> pUser2 = UserInfoMgr::getRandomUserInfo();
		std::shared_ptr<UserInfo> pUser3 = UserInfoMgr::getRandomUserInfo();
		
		sprintf(msgId,"smsgid800%d",i);

		 GroupCreate grpCreateMsg;
		 
    	grpCreateMsg.set_suserid(str_userId);
    	grpCreateMsg.set_sname("group create testing");
		grpCreateMsg.set_sremarks("group announcement");
		grpCreateMsg.set_smsgid(msgId);
		grpCreateMsg.set_npermission(atoi(str_permission));
		grpCreateMsg.set_ncreatetype(atoi(str_createtype));
		grpCreateMsg.add_sinviteuserids(pUser1? pUser1->GetUserId(): "test1");
		grpCreateMsg.add_sinviteuserids(pUser2? pUser2->GetUserId(): "test2");
		grpCreateMsg.add_sinviteuserids(pUser3? pUser3->GetUserId(): "test3");
    	CImPdu pdu;
		printf("create user=%s",str_userId);
    	pdu.SetPBMsg(&grpCreateMsg);
		pdu.SetSessionId(gSessionId);
    	//pdu.SetSessionId(atoll(sessionId)+99999900);
	    pdu.SetCommandId(GROUP_CREATE);
	  	send_to_all_server(&pdu);
		i++; 
		sleep(1);
	}
}


static void _GroupApplyJoin(void*)
{
	CConfigFileReader config_file("client.conf");
    char* str_userId = config_file.GetConfigName("userId");
	char* str_groupid = config_file.GetConfigName("groupId");
	
    
    GroupApply grpApplyMsg;
    grpApplyMsg.set_suserid(str_userId);
    grpApplyMsg.set_sgroupid(str_groupid);
	grpApplyMsg.set_smsgid("smsgid002");
	//grpApplyMsg.set_sremark("I'm Jack");

	

    CImPdu pdu;
	printf("apply group user=%s",str_userId);
    pdu.SetPBMsg(&grpApplyMsg);
	pdu.SetSessionId(gSessionId);
    pdu.SetCommandId(GROUP_APPLY);
    send_to_all_server(&pdu);
}


static void _GroupInviteJoin(void*)
{
	CConfigFileReader config_file("client.conf");
    char* fromuserId = config_file.GetConfigName("fromuid");
    char* touserId = config_file.GetConfigName("touid");
	char* str_groupid = config_file.GetConfigName("groupId");
    
    GroupInvite grpInviteMsg;
    grpInviteMsg.set_sinviterid(fromuserId);
    //grpInviteMsg.set_sinviteeid(touserId);
	grpInviteMsg.add_sinviteeids("10026");
	grpInviteMsg.add_sinviteeids("10027");	
	grpInviteMsg.add_sinviteeids("10028");
	grpInviteMsg.add_sinviteeids("10029");
	grpInviteMsg.add_sinviteeids("10030");
	grpInviteMsg.set_sgroupid(str_groupid);
	grpInviteMsg.set_smsgid("smsgid003");
	grpInviteMsg.set_sremark("I'm Jack");


    CImPdu pdu;
	printf("invite user=%s,%s",fromuserId,touserId);
    pdu.SetPBMsg(&grpInviteMsg);
	pdu.SetSessionId(gSessionId);
    pdu.SetCommandId(GROUP_INVITE);
    send_to_all_server(&pdu);
}

static void _GroupQuit(void*)
{
	CConfigFileReader config_file("client.conf");
    char* str_userId = config_file.GetConfigName("userId");
	char* str_groupid = config_file.GetConfigName("groupId");
    
    GroupQuit grpQuitMsg;
    grpQuitMsg.set_suserid(str_userId);
  	grpQuitMsg.set_sgroupid(str_groupid);
	grpQuitMsg.set_smsgid("smsgid004");


    CImPdu pdu;
	printf("quit user=%s",str_userId);
    pdu.SetPBMsg(&grpQuitMsg);
	pdu.SetSessionId(gSessionId);
    pdu.SetCommandId(GROUP_INVITE);
    send_to_all_server(&pdu);
}

static void _GroupKickout(void*)
{
	CConfigFileReader config_file("client.conf");
    char* fromuserId = config_file.GetConfigName("fromuid");
    char* touserId = config_file.GetConfigName("touid");
	char* str_groupid = config_file.GetConfigName("groupId");
    
    GroupKickOut grpKickoutMsg;
    grpKickoutMsg.set_suserid(fromuserId);
    grpKickoutMsg.set_skickid(touserId);
	grpKickoutMsg.set_sgroupid(str_groupid);
	grpKickoutMsg.set_smsgid("smsgid005");


    CImPdu pdu;
	printf("kickout user=%s,%s",fromuserId,touserId);
    pdu.SetPBMsg(&grpKickoutMsg);
	pdu.SetSessionId(gSessionId);
    pdu.SetCommandId(GROUP_KICKOUT);
    send_to_all_server(&pdu);
}


static void _GroupModify(void*)
{
	CConfigFileReader config_file("client.conf");
    char* str_userId = config_file.GetConfigName("userId");
	char* str_groupid = config_file.GetConfigName("groupId");
	char* snotifyid = config_file.GetConfigName("notifyId");
	NotifyType  nNotifyId = (NotifyType)atoi(snotifyid);    
    
    GroupModify grpModifyMsg;
	grpModifyMsg.set_suserid(str_userId);
	grpModifyMsg.set_sgroupid(str_groupid);
	grpModifyMsg.set_smsgid("smsgid006");
	grpModifyMsg.set_nmodifytype(nNotifyId);
	grpModifyMsg.set_scontent("hello world");

    CImPdu pdu;
	printf("modify user=%s,%d",str_userId,nNotifyId);
    pdu.SetPBMsg(&grpModifyMsg);
	pdu.SetSessionId(gSessionId);
    pdu.SetCommandId(GROUP_MODIFY);
    send_to_all_server(&pdu);
}

void AddBuddyAns()
{
	_AddBuddyAns(NULL);
}

string GetMsg()
{
	int msgLen = 10 + rand()% 10;
	string msg;
	while (--msgLen > 0)
	{
		msg = msg + (char)(0x21 + rand() % 0x5A);
	}
	return msg;
}


static void _ChatMsg(void* para)
{
	static int globalChatCount = 0;
	MESChat chatMsg;

    CConfigFileReader config_file(GetConfigFileName().c_str());

	char* totalcoutString = config_file.GetConfigName("tatalCount");
    char* coutString = config_file.GetConfigName("count");

    char* fromuserId = config_file.GetConfigName("userId");

    char* touserId = config_file.GetConfigName("peerUid");
    //char* msg = config_file.GetConfigName("msg");

	char* sleepStr = config_file.GetConfigName("sleepMs");
	int sleepSeconds = sleepStr ? atoi(sleepStr) : 100;
     
    string msg = getuuid();
	transform(msg.begin(), msg.end(), msg.begin(), ::toupper);	
		
    //record chat message
    chatMsg.set_sfromid(fromuserId);
	chatMsg.set_stoid(touserId);
	chatMsg.set_smsgid(msg/*"hhirwe"*/);
	chatMsg.set_msgtype(1);
	chatMsg.set_msgtime(get_tick_count());
	chatMsg.set_encrypt(0);
	chatMsg.set_scontent(GetMsg());

	unsigned long totalCount = (unsigned long)para;
	if(0 == totalCount)
		totalCount = totalcoutString ? atoi(totalcoutString) : 1;

	unsigned long currentCount = 0;

#if 1	
    while(currentCount < totalCount)
    {
		for(int i = 0; i < atoi(coutString) && currentCount < totalCount;i++)
		{
		     char szBuf[256] = {0};
        	time_t timer = time(NULL);
        	strftime(szBuf, sizeof(szBuf), "%Y-%m-%d %H:%M:%S", localtime(&timer));
			++currentCount;
			++globalChatCount;
			char content[256] = {0};
			sprintf(content,"{%s:1,%s:\"%s bulu请大家吃饭##%07d-%04lu\"}","\"contentType\"","\"content\"",szBuf, globalChatCount,currentCount);
			//sprintf(content,"##%07d-%04d",globalChatCount,currentCount);
			chatMsg.set_scontent(content);

//			char toId[256] = {0};
//			sprintf(toId,"%07d", globalChatCount);
//			chatMsg.set_stoid(toId);

			string msgId = getuuid();
			transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);
			chatMsg.set_smsgid(msgId);

			chatMsg.set_msgtime(getCurrentTime());
			
			CImPdu pdu;
			pdu.SetPBMsg(&chatMsg);
			pdu.SetSessionId(gSessionId);
			pdu.SetCommandId(MES_CHAT);   
			
			send_to_all_server(&pdu);	
//			usleep(125*1000);
		} 
		printf("Send %d messages %d times over!: \r\n",atoi(coutString), globalChatCount/*,gSessionId.c_str()*/);
		usleep(sleepSeconds*1000);
    }
#else
	CImPdu pdu;
	pdu.SetPBMsg(&chatMsg);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(MES_CHAT);   
		
	send_to_all_server(&pdu);		
#endif

}

static void _ChatCancel(void*)
{
//	static int globalChatCount = 0;
	MESChat chatMsg;

    CConfigFileReader config_file(GetConfigFileName().c_str());

    char* fromuserId = config_file.GetConfigName("userId");
    char* touserId = config_file.GetConfigName("peerUid");
    //char* msg = config_file.GetConfigName("msg");
     
    string msg = getuuid();
	transform(msg.begin(), msg.end(), msg.begin(), ::toupper);	
		
    //record chat message
    chatMsg.set_sfromid(fromuserId);
	chatMsg.set_stoid(touserId);
	chatMsg.set_smsgid(msg/*"hhirwe"*/);
	chatMsg.set_msgtype(1);
	chatMsg.set_msgtime(get_tick_count());
	chatMsg.set_encrypt(0);
	chatMsg.set_scontent(GetMsg());

	CImPdu pdu;
	pdu.SetPBMsg(&chatMsg);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(MES_CHAT);  		
	send_to_all_server(&pdu);	

	usleep(100*1000);
	
	MESChatCancel cancelMsg;
	cancelMsg.set_sfromid(fromuserId);
	cancelMsg.set_stoid(touserId);
	cancelMsg.set_smsgid(msg);
	cancelMsg.set_msgtime(chatMsg.msgtime());
	cancelMsg.set_canceltime(get_tick_count());
	CImPdu pdu2;
	pdu2.SetPBMsg(&cancelMsg);
	pdu2.SetSessionId(gSessionId);
	pdu2.SetCommandId(MES_CHATCANCEL);  	
	send_to_all_server(&pdu2);

}


static void _GrpChatMsg(void* para)
{
    CConfigFileReader config_file(GetConfigFileName().c_str());

//	char* totalcoutString = config_file.GetConfigName("tatalCount");
//  char* coutString = config_file.GetConfigName("count");

    char* fromuserId = config_file.GetConfigName("grpUserId");

    char* grpId = config_file.GetConfigName("grpId");
    //char* msg = config_file.GetConfigName("msg");

//	char* sleepStr = config_file.GetConfigName("sleepMs");
//	int sleepSeconds = sleepStr ? atoi(sleepStr) : 200;
     
    string msgId = getuuid();
	transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);	
		
    //record chat message
    
	GroupChat chatMsg;
    chatMsg.set_sfromid(fromuserId);
	chatMsg.set_sgroupid(grpId?grpId:"1");
	chatMsg.set_smsgid(msgId);
//	chatMsg.set_msgtype(1);
	chatMsg.set_msgtime(get_tick_count());
	chatMsg.set_encrypt(0);
	//chatMsg.set_scontent(GetMsg());

//	CImPdu pdu;
//	pdu.SetPBMsg(&chatMsg);
//	pdu.SetSessionId(gSessionId);
//	pdu.SetCommandId(GROUP_CHAT);   

	char* grpRepeatSendTimesStr = config_file.GetConfigName("grpRepeatSendTimes");
	unsigned long grpRepeatSendTimes = (unsigned long)para;
	if(0 == grpRepeatSendTimes)
	{
		grpRepeatSendTimes = grpRepeatSendTimesStr?atoi(grpRepeatSendTimesStr):1;
	}

	char* sleepStr = config_file.GetConfigName("grpSleepMs");
	int sleepSeconds = sleepStr ? atoi(sleepStr) : 100;

	static int count = 0;
	while(grpRepeatSendTimes > 0)
	{
		char str[128] ={'0'};
//		memset(string,0,sizeof(string));

		//sprintf(str,"{\"contentType\":1,\"content\":\"%07d\"}", ++count);

		sprintf(str,"{%s:1,%s:\"%07d\"}","\"contentType\"","\"content\"", ++count);
		//sprintf(str,"%07d", ++count);


		chatMsg.set_scontent(str);
		
		string msgId = getuuid();
		transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);
		chatMsg.set_smsgid(msgId);

		chatMsg.set_msgtime(getCurrentTime());
		
		CImPdu pdu;
		pdu.SetPBMsg(&chatMsg);
		pdu.SetSessionId(gSessionId);
		pdu.SetCommandId(GROUP_CHAT); 
		
		send_to_all_server(&pdu);
//		chatMsg.set_scontent(GetMsg());
		--grpRepeatSendTimes;
		usleep(sleepSeconds*1000);
	}
}

static void _GrpChatMsgCancel(void*)
{
    CConfigFileReader config_file(GetConfigFileName().c_str());
    char* fromuserId = config_file.GetConfigName("grpUserId");

    char* grpId = config_file.GetConfigName("grpId");
    string msgId = getuuid();
	transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);	
    
	GroupChat chatMsg;
    chatMsg.set_sfromid(fromuserId);
	chatMsg.set_sgroupid(grpId?grpId:"1");
	chatMsg.set_smsgid(msgId);
	chatMsg.set_msgtime(get_tick_count());
	chatMsg.set_encrypt(0);

	static int count = 0;
	
	char str[128] ={'0'};
	sprintf(str,"{%s:1,%s:\"%07d\"}","\"contentType\"","\"content\"", ++count);

	chatMsg.set_scontent(str);	
//	string msgId = getuuid();
//	transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);
//	chatMsg.set_smsgid(msgId);
//	chatMsg.set_msgtime(getCurrentTime());
	
	CImPdu pdu;
	pdu.SetPBMsg(&chatMsg);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(GROUP_CHAT); 
	
	send_to_all_server(&pdu);
	usleep(100*1000);

	//
	GroupChatCancel cancelMsg;
	cancelMsg.set_sfromid(fromuserId);
	cancelMsg.set_sgroupid(grpId?grpId:"1");
	cancelMsg.set_smsgid(msgId);
	cancelMsg.set_msgtime(chatMsg.msgtime());
	cancelMsg.set_canceltime(get_tick_count());
	CImPdu pdu2;
	pdu2.SetPBMsg(&cancelMsg);
	pdu2.SetSessionId(gSessionId);
	pdu2.SetCommandId(GROUP_CHATCANCEL);  	
	send_to_all_server(&pdu2);
		
}

#if 0
static void _GrpNotify(void*)
{
    CConfigFileReader config_file(GetConfigFileName().c_str());

//	char* totalcoutString = config_file.GetConfigName("tatalCount");
//  char* coutString = config_file.GetConfigName("count");

//    char* fromuserId = config_file.GetConfigName("grpUserId");

    char* grpId = config_file.GetConfigName("grpId");
	char* toId = config_file.GetConfigName("peerUid");
	
    //char* msg = config_file.GetConfigName("msg");

//	char* sleepStr = config_file.GetConfigName("sleepMs");
//	int sleepSeconds = sleepStr ? atoi(sleepStr) : 200;
     
    string msgId = getuuid();
	transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);	
		
    //record chat message
    
	MESGrpNotify grpNotify;
	grpNotify.set_sgrpid(grpId);
	grpNotify.set_stoid(toId);
	grpNotify.set_smsgid(msgId);
	grpNotify.set_msgtime(get_tick_count());
	grpNotify.set_scontent("test Notify");
	grpNotify.set_notifytype(im::NOTIFY_TYPE_GRP_ANNOUNCEMENT);
	

	CImPdu pdu;
	pdu.SetPBMsg(&grpNotify);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(MES_GRPINTERNOTIFY);   
		
	send_to_all_server(&pdu);
}
#endif

static void _JoinGrp(void*)
{
    CConfigFileReader config_file(GetConfigFileName().c_str());

    char* fromuserId = config_file.GetConfigName("userId");
    char* grpId = config_file.GetConfigName("grpId");
	char* toId = config_file.GetConfigName("peerUid");
	    
    string msgId = getuuid();
	transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);	
		  
	MESJoinGrp joinGrp;
	joinGrp.set_sfromid(fromuserId);
	joinGrp.set_sgrpid(grpId);
	joinGrp.set_stoid(toId);
	joinGrp.set_smsgid(msgId);
	joinGrp.set_reqtype(1);
	joinGrp.set_sselfintroduce("im tester");
	joinGrp.set_msgtime(get_tick_count());
	joinGrp.set_sdesc("test joinGrp");
	

	CImPdu pdu;
	pdu.SetPBMsg(&joinGrp);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(MES_JOINGRP);   
		
	send_to_all_server(&pdu);
}

static void _ExchangeGrpKey(void*)
{
    CConfigFileReader config_file(GetConfigFileName().c_str());

    char* fromuserId = config_file.GetConfigName("userId");
    char* grpId = config_file.GetConfigName("grpId");
//	char* toId = config_file.GetConfigName("peerUid");
	    
    string msgId = getuuid();
	transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);	
		  
	MESExchangeKey exchangeGrpKey;
	exchangeGrpKey.set_sfromid(fromuserId);
	exchangeGrpKey.set_sgrpid(grpId);
	exchangeGrpKey.set_smsgid(msgId);
	exchangeGrpKey.set_encrypt(0);

//	for(int i = 0; i < 1000; i++)
//	{
//		char string[25];
//		memset(string,0,sizeof(string));
//		sprintf(string,"%d", i);
//		UserKey* pKey = exchangeGrpKey.add_lsuserkeys();
//		//std::shared_ptr<UserInfo> pUserInfo =  UserInfoMgr::getRandomUserInfo();
//		pKey->set_suserid(/*pUserInfo->GetUserId()*/string);
//		pKey->set_skey("testKey");
//	}

	CImPdu pdu;
	pdu.SetPBMsg(&exchangeGrpKey);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(MES_EXCHANGE_KEY);   
		
	send_to_all_server(&pdu);
}


#if 0
static void DBALogin(void*)
{
    CConfigFileReader config_file(GetConfigFileName().c_str());

    char* str_userId = config_file.GetConfigName("userId");
    char* str_devicetype = config_file.GetConfigName("devicetype");
    char* str_msgServerIp =  config_file.GetConfigName("MsgServerIp");
    char* str_msgServerPort = config_file.GetConfigName("MsgServerPort");
    
    IM::DBA::UserLoginReq userLoginReqMsg;
    userLoginReqMsg.set_userid((uint64_t)atoll(str_userId));
    userLoginReqMsg.set_devicetype(atoi(str_devicetype));
    userLoginReqMsg.set_status(0);
    userLoginReqMsg.set_force(1);
    userLoginReqMsg.set_version("1.0");
    userLoginReqMsg.set_msgserverip(str_msgServerIp);
    userLoginReqMsg.set_msgserverport(atoi(str_msgServerPort));

    CImPdu pdu;
    pdu.SetPBMsg(&userLoginReqMsg);
    pdu.SetSessionId(123456789);
    pdu.SetCommandId(CID_DBA_LOGIN);
    send_to_all_server(&pdu);
}

static void DBALoginOutReq(void*)
{
    CConfigFileReader config_file(GetConfigFileName().c_str());

    char* str_userId = config_file.GetConfigName("userId");
    char* str_devicetype = config_file.GetConfigName("devicetype");

    
    IM::Message::LoginOutReq msg;
    msg.set_userid((uint64_t)atoll(str_userId));
    msg.set_devicetype(atoi(str_devicetype));

    CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetSessionId(123456789);
    pdu.SetCommandId(CID_MESSAGE_LOGIN_OUT_REQ);
    send_to_all_server(&pdu);
}

static void DBAModUserStatus(void*)
{
    CConfigFileReader config_file(GetConfigFileName().c_str());

    char* str_userId = config_file.GetConfigName("userId");
    char* str_devicetype = config_file.GetConfigName("devicetype");
    
    IM::DBA::ModUserStatusReq msg;
    msg.set_userid(atoll(str_userId));
    msg.set_devicetype(atoi(str_devicetype));
    msg.set_status(0);

    CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetSessionId(123456789);
    pdu.SetCommandId(CID_DBA_MOD_USER_STATUS);
    send_to_all_server(&pdu);
}

static void DBAGetUserStatusReq(void*)
{
    CConfigFileReader config_file(GetConfigFileName().c_str());

    char* str_userId = config_file.GetConfigName("userId");
    
    IM::DBA::GetUserStatusReq msg;
    msg.add_userids(atoll(str_userId));

    CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetSessionId(123456789);
    pdu.SetCommandId(CID_DBA_GET_USER_STATUS);
    send_to_all_server(&pdu);
}
#endif
static void _OfflineMsgSummary(void*)
{
	CConfigFileReader config_file(GetConfigFileName().c_str());

    char* str_userId = config_file.GetConfigName("userId");    
    MESOfflineSummary summaryMsg;
    summaryMsg.set_suserid(str_userId ? str_userId : "offline_User" );	

    CImPdu pdu;
    pdu.SetPBMsg(&summaryMsg);
    pdu.SetSessionId(gSessionId);
    pdu.SetCommandId(MES_OFFLINESUMMARY);
    send_to_all_server(&pdu);	
}

static void _OfflineMsgTotal(void*)
{
	CConfigFileReader config_file(GetConfigFileName().c_str());

    char* str_userId = config_file.GetConfigName("userId");    
	char* from_userId = config_file.GetConfigName("peerUid");  
    MESOfflineTotal msgTotal;
    msgTotal.set_stoid(str_userId ? str_userId : "3" );	
	msgTotal.set_sfromid(from_userId ? from_userId : "123456");

    CImPdu pdu;
    pdu.SetPBMsg(&msgTotal);
    pdu.SetSessionId(gSessionId);
    pdu.SetCommandId(MES_OFFLINETOTAL);
    send_to_all_server(&pdu);	
}

static void _GetOfflineMsg(void*)
{
	CConfigFileReader config_file(GetConfigFileName().c_str());

    char* str_userId = config_file.GetConfigName("hauserId");    
	char* from_userId = config_file.GetConfigName("hapeerUid1");  
    MESOfflineMsg msgReq;
    msgReq.set_stoid(str_userId ? str_userId : "1434294" );	
	msgReq.set_sfromid(from_userId ? from_userId : "1005a321bac26b4282d8dfa6ef5");

    CImPdu pdu;
    pdu.SetPBMsg(&msgReq);
    pdu.SetSessionId(gSessionId);
    pdu.SetCommandId(MES_OFFLINEMSG);
    send_to_all_server(&pdu);	
}


static void _CustomChat(void* para)
{
	static int globalChatCount = 0;
	MESChat chatMsg;

    CConfigFileReader config_file(GetConfigFileName().c_str());

	char* totalcoutString = config_file.GetConfigName("tatalCount");
    char* coutString = config_file.GetConfigName("count");

    char* fromuserId = config_file.GetConfigName("userId");

    //char* touserId = config_file.GetConfigName("peerUid");
    char touserId[256] = "10000";
    //char* msg = config_file.GetConfigName("msg");

	char* sleepStr = config_file.GetConfigName("sleepMs");
	int sleepSeconds = sleepStr ? atoi(sleepStr) : 100;
     
    string msg = getuuid();
	transform(msg.begin(), msg.end(), msg.begin(), ::toupper);	
		
    //record chat message
    chatMsg.set_sfromid(fromuserId);
	chatMsg.set_stoid(touserId);
	chatMsg.set_smsgid(msg/*"hhirwe"*/);
	chatMsg.set_msgtype(1);
	chatMsg.set_msgtime(get_tick_count());
	chatMsg.set_encrypt(0);
	chatMsg.set_scontent(GetMsg());

	unsigned long totalCount = (unsigned long)para;
	if(0 == totalCount)
		totalCount = totalcoutString ? atoi(totalcoutString) : 1;

	unsigned long currentCount = 0;

	char sServiceid[256] = {0};
	char sQuestionid[256] = {0};

#if 1	
    while(currentCount < totalCount)
    {	
    	int i = 0;
		for(; i < atoi(coutString) && currentCount < totalCount;i++)
		{
		     char szBuf[256] = {0};
        	time_t timer = time(NULL);
        	strftime(szBuf, sizeof(szBuf), "%Y-%m-%d %H:%M:%S", localtime(&timer));
			++currentCount;
			++globalChatCount;
			char content[256] = {0};
			sprintf(content,"{%s:1,%s:\"%s bulu请大家吃饭##%07d-%04lu\"}","\"contentType\"","\"content\"",szBuf, globalChatCount,currentCount);
			//sprintf(content,"##%07d-%04d",globalChatCount,currentCount);
			chatMsg.set_scontent(content);
			
			sprintf(sServiceid,"service_%d",i+1);
			sprintf(sQuestionid,"question_%d",i+1);
			
			chatMsg.set_sserviceid(sServiceid);
			chatMsg.set_squestionid(sQuestionid);
			
//			char toId[256] = {0};
//			sprintf(toId,"%07d", globalChatCount);
//			chatMsg.set_stoid(toId);

			string msgId = getuuid();
			transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);
			chatMsg.set_smsgid(msgId);

			chatMsg.set_msgtime(getCurrentTime());
			
			CImPdu pdu;
			pdu.SetPBMsg(&chatMsg);
			pdu.SetSessionId(gSessionId);
			pdu.SetCommandId(MES_CHAT);   
			
			send_to_all_server(&pdu);	
//			usleep(125*1000);
		} 
		printf("Send %d messages %d times over!: \r\n",atoi(coutString), globalChatCount/*,gSessionId.c_str()*/);
		//usleep(sleepSeconds*1);
		usleep(125*1000);
    }
#else
	CImPdu pdu;
	pdu.SetPBMsg(&chatMsg);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(MES_CHAT);   
		
	send_to_all_server(&pdu);		
#endif

}



void init_cmd_map(void)
{
#if 0
     g_cmdMap.insert(make_pair(CID_MONITOR_SERVER_REG_REQ, &RegReq));
     g_cmdMap.insert(make_pair(CID_MONITOR_SERVER_LIST_GET_REQ, &GetServerListReq));
     g_cmdMap.insert(make_pair(CID_MESSAGE_LOGIN_REQ, &_LoginReq));
     g_cmdMap.insert(make_pair(CID_MESSAGE_LOGIN_OUT_REQ, &_LoginOutReq));
     g_cmdMap.insert(make_pair(CID_MESSAGE_ADD_BUDDY, &_AddBuddy));
     g_cmdMap.insert(make_pair(CID_MESSAGE_CHAT_MSG, &_ChatMsg));    
     g_cmdMap.insert(make_pair(CID_DBA_LOGIN, &DBALogin));
     g_cmdMap.insert(make_pair(CID_MESSAGE_LOGIN_OUT_REQ, &DBALoginOutReq));
     g_cmdMap.insert(make_pair(CID_DBA_MOD_USER_STATUS, &DBAModUserStatus));
     g_cmdMap.insert(make_pair(CID_DBA_GET_USER_STATUS, &DBAGetUserStatusReq));
#else
	g_cmdMap.insert(make_pair(CM_LOGIN, &_LoginReq));
	g_cmdMap.insert(make_pair(CM_LOGOUT, &_LoginOutReq));
	g_cmdMap.insert(make_pair(SYSTEM_ASSOCSVR_REGIST, &_AssocSvrRegist));
	g_cmdMap.insert(make_pair(MES_ADDFRIEND, &_AddBuddy));
	g_cmdMap.insert(make_pair(SIG_SPONSORP2PCALL, &_SponsorCal));
	g_cmdMap.insert(make_pair(SIG_SPONSORP2PCALL_ANS, &_SponsorCalAns));	
	g_cmdMap.insert(make_pair(SIG_P2PCALL_EXCHANGE_NATINFO, &_exchangeNatInfo));	
	g_cmdMap.insert(make_pair(MES_DELFRIEND, &_DelBuddy));
	g_cmdMap.insert(make_pair(MES_CHAT, &_ChatMsg));
	g_cmdMap.insert(make_pair(MES_CHATCANCEL, &_ChatCancel));		
	g_cmdMap.insert(make_pair(MES_OFFLINESUMMARY, &_OfflineMsgSummary));	
	g_cmdMap.insert(make_pair(MES_OFFLINETOTAL, &_OfflineMsgTotal));	
	g_cmdMap.insert(make_pair(MES_OFFLINEMSG, &_GetOfflineMsg));
	g_cmdMap.insert(make_pair(GROUP_CHAT, &_GrpChatMsg));
	g_cmdMap.insert(make_pair(GROUP_CHATCANCEL, &_GrpChatMsgCancel));
	
	//g_cmdMap.insert(make_pair(MES_GRPINTERNOTIFY, &_GrpNotify));
	g_cmdMap.insert(make_pair(MES_JOINGRP,&_JoinGrp));
	g_cmdMap.insert(make_pair(MES_EXCHANGE_KEY,&_ExchangeGrpKey));
	g_cmdMap.insert(make_pair(GROUP_CREATE, &_GroupCreate));	
	g_cmdMap.insert(make_pair(GROUP_APPLY, &_GroupApplyJoin));	
	g_cmdMap.insert(make_pair(GROUP_INVITE, &_GroupInviteJoin));	
	g_cmdMap.insert(make_pair(GROUP_QUIT, &_GroupQuit));	
	g_cmdMap.insert(make_pair(GROUP_KICKOUT, &_GroupKickout));	
	g_cmdMap.insert(make_pair(GROUP_MODIFY, &_GroupModify));

	g_cmdMap.insert(make_pair(MES_CUSTOM_CHAT,&_CustomChat));
	
	
#endif
}

void send_cmd(uint32_t CmdID,void* paras)
{
    auto itr = g_cmdMap.find(CmdID);
    if (itr != g_cmdMap.end()) {
        IMCmdFunPtr fun = itr->second;
        fun(paras);
    }
}




