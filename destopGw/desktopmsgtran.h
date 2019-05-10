#ifndef _DESKTOMSGTRAN_H
#define _DESKTOMSGTRAN_H
#include <iostream>
#include "configfilereader.h"
#include "basehandle.h"
#include "loginCache.h"
//#include "CSyncData.h"

typedef struct {
    string token;
    string regid;
    string sUserId;
    ErrCode errCode;
} PCLoginAuth_t;

class DesktopMsgTran :public CBaseHandle  
{
public:
    DesktopMsgTran(CConfigFileReader* pConfigReader, int nNumOfInst);
    ~DesktopMsgTran();
    bool Initialize(void);

    void OnLogin(std::shared_ptr<CImPdu> pPdu);//pc-gw
    void LoginAck();//->pc
    void OnLogout(std::shared_ptr<CImPdu> pPdu);//pc->gw
    void OnKitout(std::shared_ptr<CImPdu> pPdu);//cm->gw
    
    void OnGwSynMessageToAppAck(std::shared_ptr<CImPdu> pPdu);//cm->gw
    void PCSynMessageToGwAck();//->pc
    void OnAppSynMessageToGw(std::shared_ptr<CImPdu> pPdu);//cm->gw
    void GwSynMessageToPC();//->pc
    void OnCheckAppActiveAck(std::shared_ptr<CImPdu> pPdu); //cm->gw
    void ReportAppActive();//->pc
    
    void OnPCSynMessageToGw(std::shared_ptr<CImPdu> pPdu);//pc->gw
    void GwSynMessageToApp();//->cm
    void OnGwSynMessageToPCAck(std::shared_ptr<CImPdu> pPdu);//pc->gw
    void AppSynMessageToGwAck();//->cm
    void OnHeartBeat(std::shared_ptr<CImPdu> pPdu);//pc->gw
    void HeartBeatAck();//->pc
    void CheckAppActive();//->cm
    
    void OnMessageFromPC(std::shared_ptr<CImPdu> pPdu);
    void OnMessageFromCM(std::shared_ptr<CImPdu> pPdu);
    //bool OnTestDesktopMsgTran(std::shared_ptr<CImPdu> pPdu);
    
    bool CheckAuth(PCLoginAuth_t &auth);
    void UpdateUserLink(UidCode_t currentSessionId,string sUserId);
protected:
    bool RegistPacketExecutor(void); 
private:
	int 			m_nNumberOfInst;

    string m_sAppSecretChecking;
    string m_sCheckAuthUrl;
    string m_sUserInfoUrl;
    CLoginCache* m_pCache;
   
    //CPackStreamQueue * m_pPackeQueue;
};

#endif
