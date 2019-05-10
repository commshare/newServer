#include"http_base.h"
#include<stdlib.h>
#include<cstring>
CHttpBase::CHttpBase()
{

}

CHttpBase::~CHttpBase()
{

}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
    if( NULL == str || NULL == buffer )
    {
        return -1;
    }
    
    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb;
}


void CHttpBase::add_multi_curl(CURLM* curl_m, const shared_ptr<HTTP_REQDATA_> httpReq, std::string& strRspData, bool isHttp2)
{
	
	struct curl_slist* pHeaders = nullptr;
	for(auto& itor : httpReq->vecHeader)
	{
		pHeaders = setHeader(pHeaders, itor);
	}

	CURL* pCurl = curl_easy_init();
	if(nullptr == pCurl)
		return;
	if(nullptr != pHeaders)
		curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
	curl_easy_setopt(pCurl, CURLOPT_URL, httpReq->strUrl_.c_str());
	setSslOption(pCurl, httpReq.get());
	//curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, std::bind(&CHttpBase::OnWriteData, this, 
	//					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)&strRspData);

	if (!httpReq->strPost_.empty())
	{
		curl_easy_setopt(pCurl, CURLOPT_POST, 1);
		curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, httpReq->strPost_.c_str());
	}
	if(isHttp2)
	{
	//	curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
	//	curl_easy_setopt(pCurl, CURLOPT_PIPEWAIT, 1L);
	}
	curl_easy_setopt(pCurl, CURLOPT_DNS_CACHE_TIMEOUT, 60 * 60 * 72);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 5);
	curl_multi_add_handle(curl_m, pCurl);
    
    m_list_curl_.push_back(pCurl);
}

void CHttpBase::setSslOption(CURL* pCurl, const P_HTTP_REQDATA_ httpReq)
{
	if(nullptr == pCurl)
		return;
	if (std::strncmp(httpReq->strUrl_.c_str(), "https://", 8) == 0)
	{
		if(!httpReq->isCert_)		// 不需要证书验证
		{
			curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		else				// 需要证书验证
		{
			curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, true);
			curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, true);
			if(!httpReq->strCainInfo_.empty())
				curl_easy_setopt(pCurl,CURLOPT_CAINFO,httpReq->strCainInfo_.c_str());
			if(!httpReq->strSslCert_.empty())
				curl_easy_setopt(pCurl,CURLOPT_SSLCERT,httpReq->strSslCert_.c_str());
			if(!httpReq->strSslKey_.empty())
				curl_easy_setopt(pCurl,CURLOPT_SSLKEY,httpReq->strSslKey_.c_str());
			if(!httpReq->strKeyPwd_.empty())
				curl_easy_setopt(pCurl,CURLOPT_KEYPASSWD,httpReq->strKeyPwd_.c_str());
		}
	}
}

bool CHttpBase::curl_multi_select(CURLM * curl_m)
{
	bool bRet = false;
	/* set a suitable timeout to play around with */ 
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	long curl_timeo = -1;
	curl_multi_timeout(curl_m, &curl_timeo);
	if(curl_timeo >= 0) 
	{
		timeout.tv_sec = curl_timeo / 1000;
		timeout.tv_usec = (curl_timeo % 1000) * 1000;
	}
	
	int maxfd = -1;
	fd_set fdread;
	FD_ZERO(&fdread);
	fd_set fdwrite;
	FD_ZERO(&fdwrite);
	fd_set fdexcep;
	FD_ZERO(&fdexcep);
	/* get file descriptors from the transfers */ 
	CURLMcode mc = curl_multi_fdset(curl_m, &fdread, &fdwrite, &fdexcep, &maxfd);
	if(mc != CURLM_OK) 
	{
		return bRet;
	}

	int rc = -1;
	if(maxfd == -1) 
	{
		struct timeval wait = { 0, 100 * 1000 }; /* 100ms */ 
		rc = select(0, NULL, NULL, NULL, &wait);
	}
	else 
	{
		rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
	}

	switch(rc) 
	{
	case -1:
		break;
	/* timeout or readable/writable sockets */ 
	case 0:
	default:
		bRet = true;
		break;
	}
	return bRet;
}

void CHttpBase::cleanHeader(struct curl_slist* pHeaders)
{
	if (nullptr != pHeaders)
	{
		curl_slist_free_all(pHeaders);
		pHeaders = nullptr;
	}
}

struct curl_slist* CHttpBase::setHeader(struct curl_slist* pHeaders, const std::string& strHead)
{
    return	(pHeaders = curl_slist_append(pHeaders, strHead.c_str()));
}

