#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstdlib>
#include "redisMsgMgr.h"
#include "redisPool.h"
#include "configfilereader.h"



void SignHandler(int iSignNo)
{
    printf("Enter SignHandler,signo:%d\r\n",iSignNo); 
//	CMongoDbManager* pMongoDbManager = CMongoDbManager::getInstance();
//	if (pMongoDbManager)
//	{
//		pMongoDbManager->Stop();
//	}
    printf("Leave SignHandler,signo:%d \r\n",iSignNo); 
}

void testRedis()
{
//	string content("test");
	CProtoMsg msg;
	msg.m_content = "test";
	msg.m_cmdId = 0;

	CReidsProtoMsgMgr::InsertProtoMsg(msg);
	//CReidsProtoMsgMgr::PopProtoMsg();

}


bool ininReidsManager()
{
	CRedisManager* pRedisDbManager = CRedisManager::getInstance();
	if (!pRedisDbManager)
	{
		ErrLog("RedisDbManager create failed");
		return false;
	}

	if (pRedisDbManager->Init("server.conf"))
	{
		ErrLog("RedisDbManager init failed");
		delete pRedisDbManager;
		return false;
	}
	return true;	
}

int main()
{
	ininReidsManager();
	
	signal(SIGINT,SignHandler); 


	srand(time(NULL));
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();
	std::vector<std::thread> threads{};
	
    //for (auto i : {0, 1, 2, 3, 4,5,6,7,8,9}) 
	std::int64_t i = 0;	
	while (++i <= 1) 
	{
        auto run = [&](std::int64_t j) 
		{
			testRedis();
        };		
        std::thread runner{run, i};
        threads.push_back(std::move(runner));
    }
	
	for (auto&& runner : threads) //首先等待threads执行完成
	{
        runner.join();
    }

	cout << "program run " << elspsedTimer.elapsed() << " usecond" << std::endl;

}

//等待结束信号


