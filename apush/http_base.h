#ifndef __HTTP_BASE_H__
#define __HTTP_BASE_H__

#include"curl.h"
#include"protobuf_phase.h"
#include<map>
#include<memory>

class CHttpBase
{
public:
	CHttpBase();
	virtual ~CHttpBase();

	virtual bool Init() = 0;

	virtual bool Regist() = 0;

	virtual void Start() = 0;

	virtual void Stop() = 0;

    virtual void AddTask(std::shared_ptr<HTTP_REQDATA_>) = 0;
	//底层留给上层的回调函数,用来处理第三方推送时,应答报文
	static void OnNotifyCallBack(void *usrData, shared_ptr<APushData> shared_APushData){}
public:
    struct curl_slist* setHeader(struct curl_slist* pHeaders, const std::string& strHead);
	void cleanHeader(struct curl_slist* pHeaders);

	void setSslOption(CURL* pCurl, const P_HTTP_REQDATA_ httpReq);
	void add_multi_curl(CURLM* curl_m, const shared_ptr<HTTP_REQDATA_> httpReq, std::string& strRspData, bool isHttp2 = false);
	bool curl_multi_select(CURLM * curl_m);

protected:
    std::list<CURL*> m_list_curl_;
};

#endif // __HTTP_BASE_H__
