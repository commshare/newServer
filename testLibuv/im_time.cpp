/*************************************************
Filename: time.cpp
Author:TongHuaizhi 			Version:1.0.0 		Date:2017/06/05
Description: 
*************************************************/
#include <stdio.h>    
#include <sys/time.h>   
#include "im_time.h"
uint64_t getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;	//将秒和微秒转换成毫秒
}

uint64_t getCurrentTime_usec()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 * 1000 + tv.tv_usec;	//将秒和微秒转换成微秒
}



CUsecElspsedTimer::CUsecElspsedTimer()
{

}

void CUsecElspsedTimer::start()
{
	m_nStartTime = getCurrentTime_usec();
}

uint64_t CUsecElspsedTimer::elapsed() const
{
	return getCurrentTime_usec() - m_nStartTime;
}

uint64_t CUsecElspsedTimer::restart()
{
	uint64_t elapsedTime = elapsed();
	start();
	return elapsedTime;
}

CElspsedTimer::CElspsedTimer()
	:CUsecElspsedTimer()
{

}

uint64_t CElspsedTimer::elapsed() const
{
	return CUsecElspsedTimer::elapsed() / 1000;
}
