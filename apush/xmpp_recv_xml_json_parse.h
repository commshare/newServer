#ifndef __XMPP_RECV_XML_JSON_PARSE_H_INCL__
#define __XMPP_RECV_XML_JSON_PARSE_H_INCL__

//#include "../tinyxml2-master/tinyxml2.h"
#include "tinyxml2/tinyxml2.h"
#include "jsoncpp/json/json.h"
#include "util.h"



using namespace tinyxml2;



/* 
<message>
<data:gcm xmlns:data="google:mobile:data">
{
"message_type":"ack",
"from":"cgLe29AKh-E:APA91bE_D3ii_kBjwOyaNngHtD8x6Bch5Vm1R63kMTGwtk5glXbw5R7TWq4lYPidYy9Bok9EEqch6fUQLVixpsDsDOnGYlHdz-acXBlJUgMuVAhFCJfXd_68Qw6iEIec9s0BBmSGHg2j",
"message_id":"46B7D99B-5CF1-5781-9241-6B7BB70--index"
}
</data:gcm></message>

<message>
<data:gcm xmlns:data="google:mobile:data">
{
"message_type":"nack",
"from":"cgLe29AKh-E:APA91bE_D3ii_kBjwOyaNngHtD8x6Bch5Vm1R63kMTGwtk5glXbw5R7TWq4lYPidYy9Bok9EEqch6fUQLVixpsDsDOnGYlHdz-acXBlJUgMuVAhFCJfXd_68Qw6iEIec9s0BBmSGHg2j",
"message_id":"46B7D99B-5CF1-5781-9241-6B7BB70--index",
"error":"INVALID_JSON",
"error_description":"Message with the same id has been received before"
}
</data:gcm>
</message> 
 */
/**
 * TODO: 解析xmpp受到的数据
 *  "message_id":"46B7D99B-5CF1-5781-9241-6B7BB70-index"
 * 注意:此处,传给fcm的messageid为消息服务器的msgId(guid),加上存储在cache中的map index
 * @author   lang
 */
class xmpp_recv_xml_json_parse
{
public:
	// Constructor
	xmpp_recv_xml_json_parse(const char *buf, int len);

	/**
	 * 成功返回true; mapIndex和msgId
	 *  
	 * @param  mapIndex, msgId
	 * 
	 * @author lang (8/31/17)
	 * 
	 * @return bool 
	 */
	bool GetIndexAndMsgId(int &mapIndex, string &msgId);

	// Destructor
	virtual ~xmpp_recv_xml_json_parse();

private:
	XMLDocument m_xmlDoc;

	int	m_iParse;
};

// Constructor implementation
inline xmpp_recv_xml_json_parse::xmpp_recv_xml_json_parse(const char *buf, int len)
{
	m_iParse = m_xmlDoc.Parse(buf, len);
}

// Destructor implementation
inline xmpp_recv_xml_json_parse::~xmpp_recv_xml_json_parse()
{
}

// TODO: Uncomment the copy constructor when you need it.
//inline xmpp_recv_xml_json_parse::xmpp_recv_xml_json_parse(const xmpp_recv_xml_json_parse& src)
//{
//   // TODO: copy
//}

// TODO: Uncomment the assignment operator when you need it.
//inline xmpp_recv_xml_json_parse& xmpp_recv_xml_json_parse::operator=(const xmpp_recv_xml_json_parse& rhs)
//{
//   if (this == &rhs) {
//      return *this;
//   }
//
//   // TODO: assignment
//
//   return *this;
//}

#endif // __XMPP_RECV_XML_JSON_PARSE_H_INCL__

