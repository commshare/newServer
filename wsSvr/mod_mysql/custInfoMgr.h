/******************************************************************************
Filename: custInfoMgr.h
Description: 获取客服信息
******************************************************************************/
#ifndef __CUSTINFOMGR_H__
#define __CUSTINFOMGR_H__

#include <string>
#include <memory>
#include <list>

using namespace std;

typedef list<string> CustList;

namespace acl
{
	class db_row;
}

class CCustInfoMgr:public Singleton<CCustInfoMgr>
{
public:
	CCustInfoMgr();
	~CCustInfoMgr();

	//获取客服号下处理questionId的客服列表
	
	bool getCustListByQuestionId(const string& questionId,const string& serviceId,CustList& custList);

private:
	bool getCustListByServiceId(const string& serviceId,CustList& custList);
	bool getCustListByQuestionId(const string& questionId,CustList& custList);
	
};
#endif // __CUSTINFOMGR_H__

