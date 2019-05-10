/******************************************************************************
Filename: grpOfflineMsg.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/04
Description: 保存群组原始消息内容
******************************************************************************/
#ifndef __GRPOFFLINEMSG_H__
#define __GRPOFFLINEMSG_H__

#include <string>
#include "mongoDbColl.h"
#include "bsoncxx/builder/basic/document.hpp"
#include "im.mes.pb.h"
#include "ostype.h"

using std::string;
using bsoncxx::document::view;

class CChannelOfflineMsg : public IMongoDataEntry
{
public:
	CChannelOfflineMsg(const im::RadioChat& msg);				
	
	string GetMsgId() { return m_sMsgId; }
	string GetFromId() { return m_sFromId; }
	string GetRadioId() {return m_sRadioId; }
	string GetContent() { return m_sContent; }
	int64_t GetMsgTime() {return m_nMsgTime; }

	virtual bsoncxx::builder::basic::document ToDoc() const override;
	virtual bool IsValid() const override;
	virtual std::shared_ptr<IMongoDataEntry> Clone()const override;
	virtual unsigned int hashVal() const override;
	virtual bsoncxx::builder::basic::document KeyDoc() const override;

private:
	string m_sFromId = "";
	string m_sRadioId = "";
	string m_sMsgId = "";
	int64_t m_nMsgTime = 0;
	int64_t m_nEncrypt = 0;
	string m_sContent = "";
	string m_sExtend = "";
	
};

#endif // __GRPOFFLINEMSG_H__
