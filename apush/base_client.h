/**
 *  file base_client.h
 *  
 */

#ifndef BASE_CLIENT_H
#define BASE_CLIENT_H

#include "util.h"
#include "protobuf_phase.h"
#include "push_provide_server.h"

typedef void(*OnNotifyCallBack_t)(void *usrData, shared_ptr<APushData> shared_APushData);

class CBaseClient
{
public:
	virtual ~CBaseClient(){};

	virtual bool Init() = 0;

	virtual bool Regist() = 0;

	virtual void Start() = 0;

	virtual void Stop() = 0;

	virtual int AddTask(shared_ptr<APushData>) = 0;

	//底层留给上层的回调函数,用来处理第三方推送时,应答报文
	static void OnNotifyCallBack(void *usrData, shared_ptr<APushData> shared_APushData);
};


#endif

