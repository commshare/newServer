#ifndef __GRPCOFFLINEMSGSEVICE_H__
#define __GRPCOFFLINEMSGSEVICE_H__

/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "im.notify.grpc.pb.h"
#include "im.radio.grpc.pb.h"
#include "singleton.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
class GrpcServer : public Singleton<GrpcServer> {
public:
    // Logic and data behind the server's behavior.
    class CRadioNotifyServiceImpl final : public radionotify::RadioNotifyService::Service {
		::grpc::Status OnRadioMsgNotify(::grpc::ServerContext* context,
										const ::radionotify::RadioMsgNotify* request,
										::radionotify::RadioMsgNotifyACK* response);
		::grpc::Status OnUserPushSetNotify(::grpc::ServerContext* context,
											const ::radionotify::UserPushSetNotify* request,
											::radionotify::UserPushSetNotifyACK* response);
	private:	
			void radioMsgNotify(const radionotify::RadioMsgNotify *request, radionotify::RadioMsgNotifyACK *response);
			void userPushSetNotify(const radionotify::UserPushSetNotify* request, radionotify::UserPushSetNotifyACK* response);
	};

	class NotifyServiceImpl final : public imnotify::NotifyService::Service {
		::grpc::Status OnGroupRelation(::grpc::ServerContext* context,
										const ::imnotify::GroupRelationNotify* request,
										::imnotify::GroupRelationNotifyACK* response);
		::grpc::Status OnFriendRelation(::grpc::ServerContext* context,
										const ::imnotify::FriendRelationNotify* request,
										::imnotify::FriendRelationNotifyACK* response);
		::grpc::Status OnLoginOperation(::grpc::ServerContext* context,
										const ::imnotify::LoginOperationNotify* request,
										::imnotify::LoginOperationNotifyACK* response);
		 ::grpc::Status OnCommonMsgNotify(::grpc::ServerContext* context,
										const ::imnotify::CommonMsgNotify* request,
										::imnotify::CommonMsgNotifyACK* response);
		private:
			void groupRelationNotify(const imnotify::GroupRelationNotify *request, imnotify::GroupRelationNotifyACK *response);
			void friendRelationNotify(const imnotify::FriendRelationNotify *request, imnotify::FriendRelationNotifyACK *response);
			void loginOperationNotify(const imnotify::LoginOperationNotify *request, imnotify::LoginOperationNotifyACK *response);
			void commonMsgNotify(const imnotify::CommonMsgNotify *request, imnotify::CommonMsgNotifyACK *response);
	};

    void RunServer(const std::string& strPath);
    void StopServer();

private:
    std::unique_ptr<Server> server;
};

#endif //__GRPCOFFLINEMSGSEVICE_H__
