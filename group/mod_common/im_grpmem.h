/******************************************************************************
Filename: im_grpmem.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/02
Description: 
******************************************************************************/
#ifndef __IM_GRPMEM_H__
#define __IM_GRPMEM_H__

#include <string>
using std::string;

bool IsGrpIdValid(const string& grpId);

class CGrpMem
{
public:
	enum GRP_MEM_STATE
	{
		GRP_MEM_STATE_QUIT_MEMBER = 0x00,	//非群成员
		GRP_MEM_STATE_APPLY_MEMBER = 0x01,		
		GRP_MEM_STATE_INVITE_MEMBER = 0x02,		
		GRP_MEM_STATE_MASTER_MEMBER = 0x03
	};
public:
	CGrpMem(const string& grpId, const string& memId, int state = 0,const string& desc="");
	const string& GetGrpId()const{ return m_grpId; }
	const string& GetMemId()const{ return m_memId; }
	const string& GetDesc()const{ return m_sDesc; }
	int GetState()const{ return m_nState; }
	void SetState(int state){ m_nState = state; }
	bool IsFlagSet(const GRP_MEM_STATE state)const { return state == m_nState; }
private:
	const string m_grpId="";
	const string m_memId="";
	int	   m_nState = 0;
	const string	m_sDesc = "";
};

#endif // __IM_GRPMEM_H__