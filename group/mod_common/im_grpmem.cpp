/******************************************************************************
Filename: im_friend.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/05
Description: 
******************************************************************************/
#include "im_grpmem.h"
#include <regex>
using std::regex;
CGrpMem::CGrpMem(const string& grpId, const string& memId, int state, const string& desc/*=""*/)
	:m_grpId(grpId), m_memId(memId), m_nState(state), m_sDesc(desc)
{

}


///////////////////////////////////////////////////////////////////
//bool IsGrpIdValid(const string& grpId)
//{
//	return true;
//	//	regex reg2("\\w*\\d+\\w*");
//	regex reg2("^(?![a-zA-Z]*$)[a-fA-F0-9]{20,28}$");
//	if (regex_match(grpId, reg2))
//	{
//		return true;
//	}
//
//	return false;
//}

bool IsGrpIdValid(const string& grpId)
{
	const int len = grpId.length();
    //if (len < 20 || len > 28) return false;
    if (len < 20 || len > 40) return false;

	int digitCount = 0;
	for (int i = 0; i < len; ++i)
	{
		const char& currentChar = grpId[i];
		if ((currentChar >= 'a' && currentChar <= 'z') || (currentChar >= 'A' && currentChar <= 'Z'))
		{
			continue;
		}
		else if (currentChar >= '0' && currentChar <= '9')
		{
			++digitCount;
		}
		else
		{
			return false;
		}
	}
	return digitCount > 0;
}
