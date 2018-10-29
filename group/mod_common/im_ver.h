/******************************************************************************
Filename: im_ver.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/09
Description: 
******************************************************************************/
#ifndef __IM_VER_H__
#define __IM_VER_H__

//英文对照： Major_Version_Number.Minor_Version_Number[Revision_Number[.Build_Number]]
#define MAJOR_VERSION "1.4"
#define MINOR_VERSION "0"

//svn version
#define REVISION "3852"
/*******************************************************************/
/*
0：初始版本
1：mongo多线程存储,离线消息的增删以任务的形式添加到多线程中
2: mongo多线程提取到基础库中，统一了回调接口
3: 初步合代码版本
4: 群组notify放到单独的线程池中处理
*/
/*******************************************************************/

#endif // __IM_VER_H__
