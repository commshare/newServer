#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include"http_base.h"
#include<queue>
#include<mutex>
#include<atomic>
#include<thread>
#include<condition_variable>

class CHttpClient : public CHttpBase
{
public:
	CHttpClient();
	~CHttpClient();
	
public:
	void addHttpData(shared_ptr<HTTP_REQDATA_> httpReq, HTTPRESPONSECALLBACK fn);
	void httpStart();

private:
	shared_ptr<HTTP_REQDATA_> getHttpData();
	void sendMultiData(CURLM* multi_handle);

private:
	std::mutex m_mtxReq_;
	std::condition_variable m_condReq_;
	std::queue<shared_ptr<HTTP_REQDATA_>> m_httpReqList_;
	
    std::atomic_bool m_run_;
	std::atomic_int m_nIndex_;
	std::mutex m_mtxCb_;
};

#endif //__HTTP_CLIENT_H__
