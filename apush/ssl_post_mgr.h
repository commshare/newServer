/**
 *  ssl_post_mgr.h
 *  
 *  
 */

#ifndef SSL_POST_MGR_H
#define SSL_POST_MGR_H

#include "util.h"
#include "ssl_socket.h"
#include "ssl_packet_queue.h"
#include "lock.h"
#include "ssl_post_impl.h"


static const int HearBeatThreadCheckNum = 50;
class CPost;

typedef struct _HeartRange
{
	void *threadParam;
	int begin;
	int end;
}HeartRange;

/*
 * Post管理类，数量监管， 心跳检测 
 * 上层回路管理 
 * threads info: 
 * 1. main thread listenning to recv logic msg , init to connect huawei push Server 
 * 2. send thread to send pushmsg to huawei push Server 
 * 3. event thread loop to check huawei recv data and the err or close event,do recv and close 
 * 4. heartbeat threads to check the connections(200), and (close)reconnct to huawei push Server 
 * @author lang (7/21/17)
 */
class CPostPoolMgr
{

	public:

	CPostPoolMgr()
	{
		m_uPostIndex = 0;
		m_uPostMaxConns = 0;
		m_uHeartBeatTimeCheck = time(NULL);

		m_uPerThreadConns = 20;
	}

	~CPostPoolMgr()
	{

	}

	/*
	static CPostPoolMgr* GetInstance()
	{

		static CPostPoolMgr *g_PostMgr = nullptr;
		if (!g_PostMgr)
		{
			g_PostMgr = new CPostPoolMgr;
		}

		return g_PostMgr;
	}
	*/

	int GetPerThreadConns(){return m_uPerThreadConns;}

	void CheckHeartBeat(HeartRange *range);

	static void* HeartBeatCheckThread(void* data);

	int GetPostDataSize();

	CPost* GetPostDataIndex(int index);

	void InsertPostData(CPost *data);

	bool Init(int uMaxConnections, OnNotifyCallBack_t fun, void* data, string host, uint16_t port);

	static void* Increase(void* data);

	int AddThreadEventConns(int MaxNum);

	int32_t Post(shared_ptr<APushData> ptrData);


private:

	CLock			m_vecMutex;
	vector<CPost *>	m_pPostVec;

	string 		m_sHost;
	uint16_t 	m_uPort;
	OnNotifyCallBack_t 	m_callBackFun;
	void* 		m_callback_data;
	int 	m_uPostIndex;
	int 	m_uPerThreadConns;
	int 	m_uPostMaxConns;

	uint32_t m_uHeartBeatTimeCheck;// = time(NULL);
};


#endif

