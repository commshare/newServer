#include"http_client.h"
#include<stdlib.h>

const int NMAXINDEX = 1000000;
const int CURL_MULTI_NUM = 32;
const int MAX_RETRY_TIMES = 3;

CHttpClient::CHttpClient()
	: m_run_(true)
	, m_nIndex_(0)
{
	
}

CHttpClient::~CHttpClient()
{
	
}

void CHttpClient::addHttpData(shared_ptr<HTTP_REQDATA_> httpReq, HTTPRESPONSECALLBACK fn)
{
	std::unique_lock<std::mutex> lock(m_mtxReq_);
    
    httpReq->reSendTimes++;
	m_httpReqList_.push(httpReq);
	m_condReq_.notify_all();
}

shared_ptr<HTTP_REQDATA_> CHttpClient::getHttpData()
{
	shared_ptr<HTTP_REQDATA_> reqData = nullptr;
	std::unique_lock<std::mutex> lock(m_mtxReq_);
	if (!m_httpReqList_.empty())
	{
		reqData = m_httpReqList_.front();
		m_httpReqList_.pop();
	}
	return reqData;
}

void CHttpClient::sendMultiData(CURLM* multi_handle)
{
	CURLMcode res = CURLM_CALL_MULTI_PERFORM;
	int still_running = 0;

	while (CURLM_CALL_MULTI_PERFORM == (res = curl_multi_perform(multi_handle, &still_running)));
	
	while(still_running)
	{
		if(curl_multi_select(multi_handle))
		{
			while (CURLM_CALL_MULTI_PERFORM == (res = curl_multi_perform(multi_handle, &still_running)));
			if (CURLM_OK != res)
			{
			    InfoLog("curl_multi_perform failure, error = %s", curl_multi_strerror(res));
			    break;
			}
		}
		else 
        {
			break;
        }
	}

	struct CURLMsg *curlMsg = nullptr;
	do
	{
		int msgq = 0;;
		curlMsg = curl_multi_info_read(multi_handle, &msgq);
		if(curlMsg) 
		{
			if(curlMsg->msg == CURLMSG_DONE)
			{
				CURL *&eh = curlMsg->easy_handle; //pay attention
				int32_t http_status_code = 0;
				const char* szUrl = nullptr;
				curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &http_status_code);
				curl_easy_getinfo(eh, CURLINFO_PRIVATE, &szUrl);
				InfoLog("get of %s returned http status code %d\n", szUrl ,http_status_code);
			}
		}
	} while(nullptr != curlMsg);

    for(auto& itor : m_list_curl_) 
    {
        curl_multi_remove_handle(multi_handle, itor);
        curl_easy_cleanup(itor);
    }	
    m_list_curl_.clear();
}

void CHttpClient::httpStart()
{
	std::thread httpThread([this]{
		CURLM *multi_handle = curl_multi_init();
		int nCount = 0;
		std::string strRspData[CURL_MULTI_NUM];
        std::vector<shared_ptr<HTTP_REQDATA_>> reqDataCach;//cach for reqdata to put back into queue when no respone
		
        while(m_run_)
		{
			shared_ptr<HTTP_REQDATA_> reqData  = getHttpData();
			if(nullptr == reqData)
			{
				 if (nCount == 0) 
				 {
					std::unique_lock<std::mutex> lock(m_mtxReq_);
					m_condReq_.wait(lock);
					continue;
				 }
				else
				{
					sendMultiData(multi_handle);
					for(int i = 0; i < nCount; ++i)
					{
						InfoLog("%d: response=%s, msgid=%s\n", i, strRspData[i].c_str(), reqDataCach[i]->msgId.c_str());
                        if (strRspData[i].empty() && reqDataCach[i]->reSendTimes < MAX_RETRY_TIMES) {
                            addHttpData(reqDataCach[i], [](void*){});
                            InfoLog("no respone, put the data back to cash for retransfer, msgid=%s, retry times = %d\n",
                                    reqDataCach[i]->msgId.c_str(), reqDataCach[i]->reSendTimes);
                        }
                    }
				    nCount = 0;
                    reqDataCach.clear();
				}
			}
			else
			{
				strRspData[nCount] = "";
				add_multi_curl(multi_handle, reqData, strRspData[nCount]);
                reqDataCach.push_back(reqData);//store into cach 
				nCount += 1;
				if(nCount >= CURL_MULTI_NUM)
				{
					sendMultiData(multi_handle);
					for(int i = 0; i < nCount; ++i)
					{
						InfoLog("%d: response=%s, msgid=%s\n", i, strRspData[i].c_str(), reqDataCach[i]->msgId.c_str());
                        if (strRspData[i].empty()) {
                            addHttpData(reqDataCach[i], [](void*){});
                            InfoLog("no respone, put the data back to cash for retransfer, msgid=%s\n", reqDataCach[i]->msgId.c_str());
                        }
					}
					nCount = 0;
                    reqDataCach.clear();
				}
				else 
                { 
					continue;
                }   
			}
		}
		curl_multi_cleanup(multi_handle);
	});
	httpThread.detach();
}
