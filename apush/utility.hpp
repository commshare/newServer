//
//  utility.hpp
//  APNS_PushServer
//
//  Created by lang on 28/06/2017.
//  Copyright © 2017 lang. All rights reserved.
//

#ifndef utility_h
#define utility_h

#include <sys/time.h>
#include <stdlib.h>
#include <string>

#include <stdio.h>
#include <string.h>

#include "base64.h"
#include "util.h"


#define PRODUCT

//#################################################HW PUSH##############################################################

static const string 	STR_HW_HOST_ADDR		= 	"login.vmall.com";
static const string 	STR_HW_HOST_ADDR_V2     =   "login.cloud.huawei.com";
static const string 	str_hw_push_addr		= "api.push.hicloud.com";

/*
"notify_type":"1",  //0=默认进入应用后拉取离线消息后进入会话列表页 
						//1=去个人聊天页面 
						//2=去群聊页面 
						//3=去联系人页面
						//4=跳转到我们的内嵌浏览器(可通过JavaScript和App交互)
	"id":"12345678",//uid或gid,取决于是去群聊还是个人聊天
	"name":"Tom",//个人名或群名,取决于是去群聊还是个人聊天
	"url":"http://www.xxx.com/xx.jpg",//个人头像或群头像或活动页面
	"extra":"unknow"//扩展字段
*/
//static string app_data_intent = "#Intent;compo=com.rvr/.Act;data=[%s]end";
#ifndef PRODUCT

	/*
	static const string 	Default_HW_Ver			= "1";
	static const string 	Default_HW_secret		= "9ace04cb63f631cfc24f5a7411560332";
	static const string 	Default_HW_clientId		= "100042631";
	static string app_data_intent = "intent:#Intent;launchFlags=0x10000000;component=com.mosant.mosantim/.module.home.SplashActivity;S.data=%s;end";
	*/

#else
	static const string 	Default_HW_Ver			= "1";
	static const string 	Default_HW_secret		= "68e0a95d497c75e25c10d7ae78e78157";
	static const string 	Default_HW_clientId		= "100143975";
	static string app_data_intent = "intent:#Intent;launchFlags=0x10000000;component=com.onlyy.kimi/com.mosant.mosantim.module.home.SplashActivity;S.data=%s;end";
#endif

static const uint16_t 	U16_CONNTIMEOUT 		= 	5000;
static const uint16_t 	U16_SENDTIMEOUT 		= 	1000;
static const uint16_t 	U16_RECVTIMEOUT 		= 	5500;

static const int16_t 	HW_HOST_PORT			= 	443;
static const string 	STR_HW_POST_HEAD		= 	"POST /oauth2/token HTTP/1.1\r\n"
													"Host: login.vmall.com\r\n"
													"Content-Type: application/x-www-form-urlencoded\r\n"
													"Content-Length: %d\r\n\n";

static const string 	STR_HW_POST_HEAD_V2		= 	"POST /oauth2/v2/token HTTP/1.1\r\n"
													"Host: login.cloud.huawei.com\r\n"
													"Content-Type: application/x-www-form-urlencoded\r\n"
													"Content-Length: %d\r\n\n";

static const uint32_t	HW_POST_CONTENT_SIZE 	= 	512;
static const string		STR_HW_POST_CONTENT 	= 	"grant_type=client_credentials&client_secret=%s&client_id=%s";

static const int32_t	HW_POST_RECV_SIZE 		= 	1024;

//{"access_token":"CFlusjjbIqNC97d69wd9fRsIC8WDy1mX7BI8v0K0j0BBBXFAiGFZ3ajXd+XCHdfCFb9ZGZM+oI\/m4BGmcwP7CQ==","expires_in":604800}
const string token = "{\"access_token\":\"";
const string expires = "\",\"expires_in\":";
const string strEnd = "}";


const static string success_respone = R"("code":"80000000")";



static const string postPushContectStr = "access_token=%s&"
										 "nsp_svc=openpush.message.api.send&"
										 "nsp_ts=%s&"
										 "device_token_list=%s&"
										 "payload=%s&"
										 "expire_time=%s&\r\n\n";

static const int max_ver_appid_size = 64;
static const string verAppid = "{\"ver\":\"%s\", \"appId\":\"%s\"}";

static const int max_head_size = 512;
static const string postPushHeadStr = "POST /pushsend.do?nsp_ctx=%s HTTP/1.1\r\n"
										"Host: api.push.hicloud.com\r\n"
										"Connection: keep-alive\r\n"
										"Content-Type: application/x-www-form-urlencoded\r\n"
										"Content-Length: ";
//static const string PostContextLen =  	"%d\r\n\n";
static const string postRequestUrl = "https://api.push.hicloud.com/pushsend.do?nsp_ctx=%s";
//###############################################HW PUSH##################################################################


//#################################################google fcm##############################################################
//RFC3920 websitehttp://blog.csdn.net/kkaxiao/article/details/6701084


static const string fcmPayLoad_to = "to";
static const string fcmPayLoad_message_id = "message_id";
static const string fcmPayLoad_data = "data";

//The maximum time to live supported is 4 weeks, and the default value is 4 weeks. 
//const string fcmPayLoad_time_to_live =  "time_to_live";
static const string fcmPayLoad_delivery_receipt_requested =  "delivery_receipt_requested";
static const string fcmPayLoad_priority =  "priority";
static const string fcmPayLoad_content_available =  "content_available";

static const string fcmPayLoad_title =  "title";
static const string fcmPayLoad_body =  "body";
static const string fcmPayLoad_notification =  "notification";

//步骤 1: 客户端初始化流给服务器:
static const string xmpp_handshake_init = "<stream:stream to='gcm.googleapis.com' version='1.0' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>";

//步骤 3: 服务器通知客户端可用的验证机制
static const string xmpp_handshake_mechanism = "<auth mechanism='PLAIN' xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>%s</auth>";

//步骤 10: 客户端发起一个新的流给服务器
static const string xmpp_handshake_stream = "<?xml version='1.0' ?><stream:stream to='gcm.googleapis.com' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'  xml:lang='en' version='1.0'>";

//发送一个流头信息,并附上任何可用的特性(iq id)
static const string xmpp_handshake_iq_num = "<iq type='set' id = '%d' xmlns='jabber:client'><bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'><resource/></bind></iq>";


//消息模板
static const string xmpp_handshake_MsgTemplate = "<message> <gcm xmlns='google:mobile:data'> %s </gcm></message>";

//粘包
static const string xmpp_msg_xml_head = "<message>";
static const string xmpp_msg_xml_tail = "</message>";

static const string xmpp_msg_success = "ack";
static const string xmpp_msg_failure = "nack";
//#######################################################google fcm#####################################################################################


//####################################################### xiao Mi Push ##################################################################################

#ifndef PRODUCT
	static const string mi_push_post_head_app_secret_default = "mo1RvQlyn7lCTeGe1i/S+g==";
	static const string mi_push_post_package_name_default = "com.mosant.mosantim";
	
//	static const string mi_push_post_form_data = "alias=%s&restricted_package_name=com.mosant.mosantim&pass_through=0&notify_type=7&notify_id=%d&title=%s&description=%s&payload=%s&extra.notify_effect=1&extra.sound_uri=android.resource://com.onlyy.kimi/raw/voip_call_in";
//     static const string mi_push_common_post_form_data = "alias=%s&restricted_package_name=com.mosant.mosantim&pass_through=0&notify_type=7&notify_id=%d&title=%s&description=%s&payload=%s&extra.notify_effect=1";
//	static const string mi_push_post_form_data = "alias=%s&restricted_package_name=com.mosant.mosantim&pass_through=0&notify_type=7&title=%s&description=%s&payload=%s&extra.notify_effect=1&extra.sound_uri=android.resource://com.onlyy.kimi/raw/voip_call_in";
	static const string mi_push_post_form_data =        "alias=%s&restricted_package_name=com.mosant.mosantim&pass_through=0&notify_type=7&notify_id=%d&title=%s&description=%s&payload=%s&extra.notify_effect=1";
	static const string mi_push_common_post_form_data = "alias=%s&restricted_package_name=com.mosant.mosantim&pass_through=0&notify_type=7&notify_id=%d&title=%s&description=%s&payload=%s&extra.notify_effect=1";

#else
	static const string mi_push_post_head_app_secret_default = "2QuNEZ7GxNAlxSHnl1LSZA==";
	static const string mi_push_post_package_name_default = "com.onlyy.kimi";
	
//	static const string mi_push_post_form_data = "alias=%s&restricted_package_name=com.onlyy.kimi&pass_through=0&notify_type=7&notify_id=%d&title=%s&description=%s&payload=%s&extra.notify_effect=1&extra.sound_uri=android.resource://com.onlyy.kimi/raw/voip_call_in";
//	static const string mi_push_common_post_form_data = "alias=%s&restricted_package_name=com.onlyy.kimi&pass_through=0&notify_type=7&notify_id=%d&title=%s&description=%s&payload=%s&extra.notify_effect=1";
//	static const string mi_push_post_form_data = "alias=%s&restricted_package_name=com.onlyy.kimi&pass_through=0&notify_type=7&title=%s&description=%s&payload=%s&extra.notify_effect=1&extra.sound_uri=android.resource://com.onlyy.kimi/raw/voip_call_in";
	static const string mi_push_post_form_data =        "alias=%s&restricted_package_name=com.onlyy.kimi&pass_through=0&notify_type=7&notify_id=%d&title=%s&description=%s&payload=%s&extra.notify_effect=1";
	static const string mi_push_common_post_form_data = "alias=%s&restricted_package_name=com.onlyy.kimi&pass_through=0&notify_type=7&notify_id=%d&title=%s&description=%s&payload=%s&extra.notify_effect=1";
#endif

static const string mi_push_post_head = "POST /v2/message/alias HTTP/1.1\r\n"
		"authorization: key=2QuNEZ7GxNAlxSHnl1LSZA==\r\n"
		"Host: api.xmpush.xiaomi.com\r\n"
		"restricted_package_name=com.onlyy.kimi\r\n"
		//"Accept: */*\r\n"
		"Content-Type:  application/x-www-form-urlencoded\r\n"
		"Content-Length: %d\r\n"
		"\n%s\r\n";

//######################################################### xiao mi Push ###############################################################################

//############################################# JPush###############################################

static const string jpush_appKey_default = "3e42ab82727d00a206a7cd05";
static const string jpush_masterSecret_default = "c4b7eaba56fe631d3b80a489";
static const string jpush_webSit = "api.jpush.cn";
static const string jpush_head = "POST /v3/push  HTTP/1.1\r\n"
		"authorization: Basic %s\r\n"
		"Host: api.jpush.cn\r\n"
		//"Accept: */*\r\n"
		"Content-Type:  application/x-www-form-urlencoded\r\n"
		"Content-Length: %d\r\n"
		"\n%s\r\n";;
//############################################# JPush###############################################




/**
 获取当前系统时间的毫秒级

 @return 系统毫米时间 
 */
//static unsigned long GetTickTime()
//{
//	struct  timeval    tv;
//	
//	gettimeofday(&tv, nullptr);
//
//	return (tv.tv_sec*1000 + tv.tv_usec/1000);
//
//}

//
//static void S_usleep(unsigned int usec)
//{
//	unsigned int sec = usec/(1000000);
//	unsigned int utmpsec = 0;
//	if (sec > 0)
//	{
//		utmpsec = sec % 1000000;
//	}
//	struct  timeval    tv{sec, utmpsec};
//
//	select(0, NULL, NULL, NULL, &tv);
//}

inline int SetBlockOrNot( int fd, int flag /*= 0*/ )
{
		int opts;
		opts=fcntl(fd,F_GETFL);
		if(opts<0)
		{
				return -1;
		}

		if (flag == 0)
		{
				opts &= ~O_NONBLOCK;
		}
		else
		{
				opts = opts|O_NONBLOCK;
		}

		if(fcntl(fd,F_SETFL,opts)<0)
		{
				return -1;
		}
		return 0;
}

std::string rfc1738_encode(const std::string& src, bool bigOrLittle = false);

#endif /* utility_hpp */

