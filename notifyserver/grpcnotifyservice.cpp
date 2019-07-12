#include "grpcnotifyservice.h"
#include"pdusender.h"
#include "configfilereader.h"
#include "util.h"



void GrpcServer::NotifyServiceImpl::groupRelationNotify(const imnotify::GroupRelationNotify *request, imnotify::GroupRelationNotifyACK *response)
{
	DbgLog("group relation msg=%s grpid=%s type=%d from=%s operId=%s", request->smsgid().c_str(), request->sgrpid().c_str(), request->notifytype(), request->sopruserid().c_str(), request->soperid().c_str());
	im::SVRMSGGroupRelationNotify groupNotify;
	groupNotify.set_sgrpid(request->sgrpid());
	groupNotify.set_smsgid(request->smsgid());
	groupNotify.set_msgtime(request->msgtime());
	groupNotify.set_scontent(request->scontent());
	groupNotify.set_notifytype((im::SVRGroupRelationNotifyType)request->notifytype());
	groupNotify.set_extend(request->extend());
	groupNotify.set_sopruserid(request->sopruserid());
	groupNotify.set_soperid(request->soperid());
	for (int i = 0; i < request->smnpleduserid_size(); i++)
	{
		groupNotify.add_smnpleduserid(request->smnpleduserid(i));
	}

	for(int i = 0; i < request->stoids_size(); ++i)
	{
		groupNotify.add_stoids(request->stoids(i));
	}
	
	CPduSender::getInstance()->sendReq(&groupNotify, im::SVR_GROUP_RELATIN_NOTIFY, imsvr::MSG);
	
	response->set_smsgid(request->smsgid());
	response->set_msgtime(getCurrentTime());
	response->set_expcode(imnotify::ExceptionCode::NO_EXP);
	DbgLog("group relation ack  msg=%s grpid=%s type=%d from=%s operId=%s", request->smsgid().c_str(), request->sgrpid().c_str(), request->notifytype(), request->sopruserid().c_str(), request->soperid().c_str());
}

void GrpcServer::NotifyServiceImpl::friendRelationNotify(const imnotify::FriendRelationNotify *request, imnotify::FriendRelationNotifyACK *response)
{
	DbgLog("friend relation msg=%s from=%s to=%s type=%d", request->smsgid().c_str(), request->sfromid().c_str(), request->stoid().c_str(), request->notifytype());
	SVRMSGFriendRelationNotify friendNotify;
	friendNotify.set_sfromid(request->sfromid());
	friendNotify.set_stoid(request->stoid());
	friendNotify.set_msgtime(request->msgtime());
	friendNotify.set_smsgid(request->smsgid());
	friendNotify.set_smemoname(request->smemoname());
	friendNotify.set_sselfintroduce(request->sselfintroduce());
	friendNotify.set_extend(request->extend());
	friendNotify.set_soperid(request->soperid());
	friendNotify.set_notifytype((im::SVRFriendRelationNotifyType)request->notifytype());
	CPduSender::getInstance()->sendReq(&friendNotify, im::SVR_FRIEND_RELATION_NOTIFY, imsvr::MSG);
	
	response->set_smsgid(request->smsgid());
	response->set_msgtime(getCurrentTime());
	response->set_expcode(imnotify::ExceptionCode::NO_EXP);
	DbgLog("friend relation ack msg=%s from=%s to=%s type=%d", request->smsgid().c_str(), request->sfromid().c_str(), request->stoid().c_str(), request->notifytype());
}

void GrpcServer::NotifyServiceImpl::loginOperationNotify(const imnotify::LoginOperationNotify *request, imnotify::LoginOperationNotifyACK *response)
{
	DbgLog("login operation userid=%s, type=%d", request->suserid().c_str(), request->logintype());
	CMPHPLoginNotify loginNotify;
	loginNotify.set_suserid(request->suserid());
	loginNotify.set_msgtime(request->msgtime());
	loginNotify.set_logintype((::im::PHPLoginNotifyType)request->logintype());
	loginNotify.set_sdevicetoken(request->sdevicetoken());
	loginNotify.set_extend(request->extend());
	loginNotify.set_loginsubtype((::im::PHPLoginNotifySubType)request->loginsubtype());

	//CImPdu	   loginNotifyPdu;
	//loginNotifyPdu.SetPBMsg(&loginNotify);
	//loginNotifyPdu.SetCommandId(im::CM_PHP_LOGIN_NOTIFY);
	//SendPdu(CM,&loginNotifyPdu);
	CPduSender::getInstance()->sendReq(&loginNotify, im::CM_PHP_LOGIN_NOTIFY, imsvr::LOGIN);

	response->set_suserid(request->suserid());
	response->set_msgtime(getCurrentTime());
	response->set_expcode(imnotify::ExceptionCode::NO_EXP);
	DbgLog("login operation ack userid=%s", request->suserid().c_str());
}

void GrpcServer::NotifyServiceImpl::commonMsgNotify(const imnotify::CommonMsgNotify *request, imnotify::CommonMsgNotifyACK *response)
{
	DbgLog("common notify msg=%s from=%s type=%d", request->smsgid().c_str(), request->sfromid().c_str(), request->notifytype());
	SVRMSGCommonMsgNotify commNotify;
	commNotify.set_sfromid(request->sfromid());
	commNotify.set_smsgid(request->smsgid());
	commNotify.set_scontent(request->scontent());
	commNotify.set_msgtime(request->msgtime());
	commNotify.set_notifytype((im::SVRCommonNotifyType)request->notifytype());
	for (int i = 0; i < request->stoids_size(); i++)
	{
		commNotify.add_stoids(request->stoids(i));			
	}

	CPduSender::getInstance()->sendReq(&commNotify, im::SVR_COMMON_MSG_NOTIFY, imsvr::MSG);
	response->set_smsgid(request->smsgid());
	response->set_msgtime(getCurrentTime());
	response->set_expcode(imnotify::ExceptionCode::NO_EXP);
	DbgLog("common notify ack msg=%s from=%s type=%d", request->smsgid().c_str(), request->sfromid().c_str(), request->notifytype());
}

grpc::Status GrpcServer::NotifyServiceImpl::OnGroupRelation(grpc::ServerContext *context, const imnotify::GroupRelationNotify *request, imnotify::GroupRelationNotifyACK *response)
{
	cout << "=======OnGroupRelation=======" <<endl;
	groupRelationNotify(request, response);
	return Status::OK;
}

grpc::Status GrpcServer::NotifyServiceImpl::OnFriendRelation(grpc::ServerContext *context, const imnotify::FriendRelationNotify *request, imnotify::FriendRelationNotifyACK *response)
{
    cout << "------OnFriendRelation--------" << endl;
	friendRelationNotify(request, response);
    return Status::OK;
}

grpc::Status GrpcServer::NotifyServiceImpl::OnLoginOperation(grpc::ServerContext *context, const imnotify::LoginOperationNotify *request, imnotify::LoginOperationNotifyACK *response)
{
	cout << "=========OnLoginOperation==========" <<endl;
	loginOperationNotify(request, response);
	return Status::OK;
}

grpc::Status GrpcServer::NotifyServiceImpl::OnCommonMsgNotify(::grpc::ServerContext* context, const ::imnotify::CommonMsgNotify* request, ::imnotify::CommonMsgNotifyACK* response)
{
	cout << "=========OnCommonMsgNotify==========" <<endl;
	commonMsgNotify(request, response);
	return Status::OK;
}


grpc::Status GrpcServer::CRadioNotifyServiceImpl::OnRadioMsgNotify(::grpc::ServerContext* context, const ::radionotify::RadioMsgNotify* request, ::radionotify::RadioMsgNotifyACK* response)
{
	cout << "=========OnRadioMsgNotify==========" <<endl;
	radioMsgNotify(request, response);
	return Status::OK;
}

::grpc::Status GrpcServer::CRadioNotifyServiceImpl::OnRadioPushSetNotify(::grpc::ServerContext* context, const ::radionotify::RadioPushSetNotify* request, ::radionotify::RadioPushSetNotifyACK* response)
{
	cout << "=========OnUserPushSetNotify==========" <<endl;
	radioPushSetNotify(request, response);
	return Status::OK;
}


void GrpcServer::CRadioNotifyServiceImpl::radioMsgNotify(const radionotify::RadioMsgNotify *request, radionotify::RadioMsgNotifyACK *response)
{
	DbgLog("radio relation msg=%s radio=%s type=%d from=%s", request->smsgid().c_str(), request->sradioid().c_str(), request->notifytype(), request->sopruserid().c_str());
	im::SVRRadioMsgNotify radioNotify;
	radioNotify.set_sradioid(request->sradioid());
	radioNotify.set_smsgid(request->smsgid());
	radioNotify.set_msgtime(request->msgtime());
	radioNotify.set_scontent(request->scontent());
	radioNotify.set_notifytype((im::SVRRadioNotifyType)request->notifytype());
	radioNotify.set_extend(request->extend());
	radioNotify.set_sopruserid(request->sopruserid());
	for (int i = 0; i < request->smnpleduserid_size(); i++)
	{
		radioNotify.add_smnpleduserid(request->smnpleduserid(i));
	}
	
	CPduSender::getInstance()->sendReq(&radioNotify, im::SVR_RADIO_RELATIN_NOTIFY, imsvr::CHANNEL);
	
	response->set_smsgid(request->smsgid());
	response->set_msgtime(getCurrentTime());
	response->set_expcode(radionotify::ExceptionCode::NO_EXP);
	DbgLog("radio relation ack  msg=%s radio=%s type=%d from=%s", request->smsgid().c_str(), request->sradioid().c_str(), request->notifytype(), request->sopruserid().c_str());
}

void GrpcServer::CRadioNotifyServiceImpl::radioPushSetNotify(const radionotify::RadioPushSetNotify* request, radionotify::RadioPushSetNotifyACK* response)
{
	DbgLog("user push set msg=%s user=%s radio=%s push=%d status=%d", request->smsgid().c_str(), request->suserid().c_str(), request->sradioid().c_str(), request->notifytype(), request->status());
	im::SVRRadioPushSetNotify radioNotify;
	radioNotify.set_smsgid(request->smsgid());
	radioNotify.set_suserid(request->suserid());
	radioNotify.set_sradioid(request->sradioid());
	radioNotify.set_notifytype((im::SVRRadioPushSetNotifyType)request->notifytype());
	radioNotify.set_status(request->status());
	radioNotify.set_msgtime(request->msgtime());
	if(radionotify::PUSH_NEWMSG == request->notifytype() || radionotify::PUSH_HIDEMSGSOUNDON == request->notifytype())
		CPduSender::getInstance()->sendReq(&radioNotify, im::SVR_RADIO_PUSHSET_NOTIFY, imsvr::LOGIN);
	else
		CPduSender::getInstance()->sendReq(&radioNotify, im::SVR_RADIO_PUSHSET_NOTIFY, imsvr::CHANNEL);
	
	response->set_smsgid(request->smsgid());
	response->set_msgtime(getCurrentTime());
	response->set_expcode(radionotify::ExceptionCode::NO_EXP);
	DbgLog("user push set ack  msg=%s user=%s radio=%s push=%d status=%d", request->smsgid().c_str(), request->suserid().c_str(), request->sradioid().c_str(), request->notifytype(), request->status());
}





void GrpcServer::RunServer(const std::string& strPath)
{
    ServerBuilder builder;
	
	CConfigFileReader configRead(strPath.c_str());
	char* pSvr =  configRead.GetConfigName("grpc_svr_host");
	DbgLog("Server listening on: %s", pSvr);
    NotifyServiceImpl notifyservice;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(pSvr, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&notifyservice);

//	pSvr =  configRead.GetConfigName("radio_svr_host");
//	DbgLog("radio server listening on: %s", pSvr);
	CRadioNotifyServiceImpl radioSvr;
//	builder.AddListeningPort(pSvr, grpc::InsecureServerCredentials());
	builder.RegisterService(&radioSvr);
	
    // Finally assemble the server.
    server = builder.BuildAndStart();

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.

    server->Wait();
}

void GrpcServer::StopServer() {

    server->Shutdown();
}
