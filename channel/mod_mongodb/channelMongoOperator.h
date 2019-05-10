#ifndef _H_CHANNEL_MONGO_OP_
#define _H_CHANNEL_MONGO_OP_
#include <string>
#include <set>
#include<vector>
#include "channel_offline_msg.h"
#include "singleton.h"

using namespace std;
class ChannelMongoOperator : public Singleton<ChannelMongoOperator>
{
public:
	ChannelMongoOperator();
	~ChannelMongoOperator();

public:
	void initCurrDbName(const string& dbName);
	void checkAndCreateCollection(const std::string& collectionName);

	bool CreateCollection(const string& collectionName);
	bool DelCollection(const string& collectionName);
	set<string> GetCollections();
	bool CreateIndex(const string& collectionName, const string& indexName, bool unique, bool backGround = true);
	bool IsExistCollection(string& collectionName);
	bool HasCollection(const string& collectionName);

	string InsertChannelOfflineMsg(const CChannelOfflineMsg& offlineMsg, const string& collectionName);
	int UpdateChannelOfflineMsg(const std::string& collectionName, const std::string& msgId, const std::string& extend);
	bool getChannelOfflineMsg(const std::string& collectionName, std::vector<std::string>& vecResult, int limitNum);
	string GetChannelOfflineMsg(const std::string& collectionName, const std::string& msgId);
	bool DelOfflineMsg(const string& collectionName, const string& msgId);

    

    
private:
    string m_sAdminDbName;                   //admin db for run command     
    string m_sCurrDbName;
	std::set<std::string> m_mongoCollections;
};

#endif//
