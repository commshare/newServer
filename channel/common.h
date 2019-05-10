#ifndef __COMMON_H__
#define __COMMON_H__

#include<string>
#include<vector>
#include<map>
using namespace std;

typedef struct
{
	string userId;
	int status;
	string strIp;
	int nPort;
	int nPushType;
	string pushToken;
}USER_LOGIN_INIFO_, *P_USER_LOGIN_INIFO_;

enum CHNN_STATUS_
{
	CHNN_UNKOWN = 0,
	CHNN_DEPOSIT,		// 未激活
	CHNN_NORMAL,			// 正常
	CHNN_DISMISS			// 解散
};

typedef struct struct_chnn_info_
{
	string radioId;
	CHNN_STATUS_ status;
	int unspeak;
	struct_chnn_info_()
	{
		radioId = "";
		status = CHNN_UNKOWN;
		unspeak = -1;
	}
}CHANNEL_INIFO_, *P_CHANNEL_INIFO_;

#endif // __COMMON_H__