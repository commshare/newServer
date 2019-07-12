/******************************************************************************
Filename: im_time.h
Author:TongHuaizhi 			Version:1.0.0 		Date:2017/06/05
Description: 时间相关的函数
******************************************************************************/
#ifndef __IM_TIME_H__
#define __IM_TIME_H__
#include "ostype.h"


uint64_t getCurrentTime();			//获取系统的当前时间（毫秒ms）
uint64_t getCurrentTime_usec();		//获取系统的当前时间（微秒us）

class CUsecElspsedTimer
{
public:
	CUsecElspsedTimer();
	bool isValid() const{ return m_nStartTime != 0; }
	void start();
	virtual uint64_t elapsed()const;
	uint64_t restart();
private:
	uint64_t m_nStartTime = 0;
};

class CElspsedTimer : public CUsecElspsedTimer
{
public:
	CElspsedTimer();
	virtual uint64_t elapsed() const override;

};
#endif // __IM_TIME_H__
