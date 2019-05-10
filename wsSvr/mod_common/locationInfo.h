/******************************************************************************
Filename: location_info.h
Description: 
******************************************************************************/
#ifndef __LOCATION_INFO_H__
#define __LOCATION_INFO_H__

#include <string>
#include "ostype.h"
using std::string;

class  CLocationInfo
{
public:
	CLocationInfo(uint32_t id, uint32_t pid,const string& path, uint32_t level, const string& name);
	CLocationInfo();
	 ~CLocationInfo();

public:
	uint32_t        m_nId;  //自己ID
	uint32_t		m_nPid; //父ID
	string		    m_sPath; //路径
	uint32_t		m_nLevel; //层级
	string		    m_sName;  //中文名称
};
#endif // __LOCATION_INFO_H__

