#include "xmpp_recv_xml_json_parse.h"
#include "utility.hpp"

// TODO: your implementation here
bool xmpp_recv_xml_json_parse::GetIndexAndMsgId(int &mapIndex, string &msgId)
{

	bool bRet = false;

	//处理xml
	if (m_iParse != 0)
	{
		return false;
	}

	XMLElement *titleElement = m_xmlDoc.FirstChildElement("message")->FirstChildElement("data:gcm");
	XMLText* textNode = titleElement->FirstChild()->ToText();
	const char *payLoadStr = textNode->Value();

	if (!payLoadStr)
	{
		ErrLog("textNode->Value()");
		return false;
	}
	

	//处理json
	Json::Reader reader;  
    Json::Value root;

	bRet = reader.parse(payLoadStr, root);
	if(!bRet)
	{
		ErrLog("eader.parse(payLoadStr, root):%s", payLoadStr);
		return false;
	}

	string message_type = root["message_type"].asString(); 
	if (message_type == xmpp_msg_success)
	{
		bRet = true;
	}
	else
	{
		bRet = false;

		WarnLog("error:%s, error_description:%s", root["error"].asString().c_str(), 
				root["error_description"].asString().c_str());
	}

	string message_id = root["message_id"].asString();
	if (message_id.size() < 34)
	{
		//iRet = false;
		return false;
	}

	//"46B7D99B-5CF1-5781-9241-6B7BB70--"

	//printf("msgId:%s\n", message_id.c_str());

	size_t pos = message_id.find("--");
	msgId = message_id.substr(0, pos);
	//msgId += '\0';
	
	string index = message_id.substr(pos+2);
	//printf("index:%s\n", index.c_str());

	mapIndex = std::stoi(index);

	return bRet;
}
