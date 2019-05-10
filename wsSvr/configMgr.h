/******************************************************************************
Filename: configMgr.h
autho:bob
date:2018-12-18
description:配置文件管理类
******************************************************************************/
#ifndef _CONFIGMGR_H__
#define _CONFIGMGR_H__

#include "configfilereader.h"
#include "singleton.h"

class CConfigMgr:public Singleton<CConfigMgr>
{
private:
	CConfigMgr();
    
public:
	virtual ~CConfigMgr();
	void init(const char * pConfigFile);

	CConfigFileReader* 	m_pConfigReader;
};

#endif //_CONFIGMGR_H__

