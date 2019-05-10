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
#include<unistd.h>
#include <grpcpp/grpcpp.h>

#include "im.notify.grpc.pb.h"
#include "im_time.h"
#include <chrono>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
//using imnotify::MESOfflineMsg;
//using imnotify::MESOfflineMsgAck;
//using imnotify::OfflineMsgService;
using imnotify::GroupRelationNotify;
using imnotify::GroupRelationNotifyACK;
using imnotify::FriendRelationNotify;
using imnotify::FriendRelationNotifyACK;
using imnotify::LoginOperationNotify;
using imnotify::LoginOperationNotifyACK;
using imnotify::NotifyService;
using imnotify::NotifyType;
using imnotify::FriendNotifyType;
using namespace  std;


class OfflineMsgClient {
public:
  OfflineMsgClient(std::shared_ptr<Channel> channel)
//      : stub_(OfflineMsgService::NewStub(channel)),
       : stub_notify(NotifyService::NewStub(channel)){}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
//  MESOfflineMsgAck OnOfflineMsg(const std::string &fromId,  const std::string &toId, int cmdId, int cnt) {
//      MESOfflineMsg  request;
//     request.set_sfromid(fromId);
//      request.set_stoid(toId);
//      request.set_cmdid(cmdId);
//      request.set_count(cnt);
//      request.set_smsgid("aafddeiefeife1215f92f");
//      request.clear_lsmsgs();

//      MESOfflineMsgAck  reply;
//      ClientContext context;
//      Status status = stub_->OnOfflineMsg(&context, request, &reply);

//      if (status.ok()) {
//          std::cout << "reply... ok" << std::endl;
//      } else {
//        std::cout << status.error_code() << ": " << status.error_message()
//                 << std::endl;
      //  return MESOfflineMsgAck();
//      }
//      return reply;
//    }

//  MESOfflineMsgAck OnOfflineMsg(MESOfflineMsg& request) {


//      MESOfflineMsgAck  reply;
//      ClientContext context;
//      Status status = stub_->OnOfflineMsg(&context, request, &reply);

//      if (status.ok()) {
//          std::cout << "reply... ok" << std::endl;
//      } else {
//        std::cout << status.error_code() << ": " << status.error_message()
//                  << std::endl;
      //  return MESOfflineMsgAck();
//      }
//      return reply;
//    }

//  std::string OnOfflineMsg(const std::string& user) {
    // Data we are sending to the server.
//    MESOfflineMsg  request;
//    request.set_sfromid("");
//    request.set_stoid("4071819");
//    request.set_cmdid(0);
//    request.set_count(100);
//    request.set_smsgid("aafddeiefeife1215f92f");
//    request.clear_lsmsgs();
//    request.set_name(user);

    // Container for the data we expect from the server.
//    MESOfflineMsgAck  reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
//    ClientContext context;

    // The actual RPC.
//    Status status = stub_->OnOfflineMsg(&context, request, &reply);

    // Act upon its status.
//    if (status.ok()) {
//      return reply.message();
//	    std::cout << "reply... ok" << std::endl;
//    } else {
//      std::cout << status.error_code() << ": " << status.error_message()
//                << std::endl;
//      return "RPC failed";
//    }
//  }

  std::string OnGroupRelationNotify() {
        GroupRelationNotify grpRelationNotify;
        grpRelationNotify.set_sgrpid("abcdefghijklmnopqrst0123456789");
        //for(int j = 0; j<5; j++) {
        //  string *tmp = grpRelationNotify.add_lstoid();
        //  char buf[4] ={0};
        //  memset(buf, 0, 4);
        //  sprintf(buf, "%d", j);
        //  *tmp = string("user_") + buf;
        //}
        grpRelationNotify.set_smsgid("skdfjigirgirjgrjggjjg");
        grpRelationNotify.set_msgtime(getCurrentTime());
        grpRelationNotify.set_sopruserid("405588");
        grpRelationNotify.set_notifytype(NotifyType::NOTIFY_TYPE_GRPINFO_CHANGED);

        GroupRelationNotifyACK  reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;
		std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(10*1000);
		context.set_deadline(deadline);
        // The actual RPC.
		uint64_t now = getCurrentTime();
        Status status = stub_notify->OnGroupRelation(&context, grpRelationNotify, &reply);
		printf("111111111111111111111111111111111111111  time:%lu\n", getCurrentTime() - now);
        // Act upon its status.
        if (status.ok()) {
    //      return reply.message();
            cout << "------GroupRelationNotifyACK----" << endl;
            std::cout << "reply... ok" << std::endl;
            cout << "msgtime: " << reply.msgtime() <<endl;
            cout << "msgid: " << reply.smsgid() << endl;
            cout << "expcode: " << reply.expcode() << endl;
            cout << "------GroupRelationNotifyACK finished----"<< endl;
            return "RPC successfully";
        } else {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
          return "RPC failed";
        }
		printf("22222222222222222222222222222222222\n");
   }


  std::string OnFriendRelationNotify() {
        FriendRelationNotify frdRelationNotify;

        frdRelationNotify.set_sfromid("6366545");
        frdRelationNotify.set_stoid("1111111");
        frdRelationNotify.set_smsgid("1234354686983458999");
        frdRelationNotify.set_smemoname("xiaoming");
        frdRelationNotify.set_notifytype(FriendNotifyType::FRIEND_NOTIFY_TYPE_ADD);
        frdRelationNotify.set_sselfintroduce("bad guy");
        frdRelationNotify.set_msgtime(getCurrentTime());

        FriendRelationNotifyACK  reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_notify->OnFriendRelation(&context,
                                                      frdRelationNotify,
                                                      &reply);

        // Act upon its status.
        if (status.ok()) {
    //      return reply.message();
            cout << "------FriendRelationNotifyACK----" << endl;
            std::cout << "reply... ok" << std::endl;
            cout << "msgtime: " << reply.msgtime() <<endl;
            if(!reply.smsgid().empty()) {
                cout << "msgid: " << reply.smsgid() << endl;
            }
            cout << "expcode: " << reply.expcode() << endl;
            return "RPC successfully";
        } else {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
          return "RPC failed";
        }
   }

  std::string OnOnLoginOperationNotify() {
        LoginOperationNotify loginoperationotify;

        loginoperationotify.set_suserid("6366545");
        loginoperationotify.set_msgtime(getCurrentTime());

        LoginOperationNotifyACK  reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_notify->OnLoginOperation(&context,
                                                      loginoperationotify,
                                                      &reply);

        cout << "-------OnLoginOperation finished------" << endl;
        // Act upon its status.
        if (status.ok()) {
    //      return reply.message();
            cout << "--------LoginOperationNotifyACK------" << endl;
            std::cout << "reply... ok" << std::endl;
            cout << "msgtime: " << reply.msgtime() <<endl;
            cout << "expcode: " << reply.expcode() << endl;
            return "RPC successfully";
        } else {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
          return "RPC failed";
        }
   }


 private:
//  std::unique_ptr<OfflineMsgService::Stub> stub_;
  std::unique_ptr<NotifyService::Stub> stub_notify;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  OfflineMsgClient client(grpc::CreateChannel(
      "192.168.1.56:3400", grpc::InsecureChannelCredentials()));
//  std::string user("4071819");//dora
//  std::string user("4071468");//sam
//  //MESOfflineMsgAck reply = client.OnOfflineMsg("", user, 0, 200);
//  MESOfflineMsg  request;
//  request.set_sfromid("");
//  request.set_stoid(user);
//  request.set_cmdid(0);
//  request.set_count(200);
//  request.set_smsgid("aafddeiefeife1215f92f");
//  request.clear_lsmsgs();
//暂时注释
//  MESOfflineMsgAck reply = client.OnOfflineMsg(request);
//  std::cout << "offlinemsgservice received: " << reply.msglist().size() << std::endl;
//  request.clear_lsmsgs();
//  //imnotify::OfflineDeliveredMsg* pMsg = request.add_lsmsgs();
//  for(int i = 0; i < reply.msglist_size(); i++) {
//      imnotify::OfflineDeliveredMsg* pMsg = request.add_lsmsgs();
//      pMsg->set_cmdid(reply.msglist(i).cmdid());
//      pMsg->set_sfromid(reply.msglist(i).sfromid());
//      pMsg->set_stoid(user);
//      pMsg->set_smsgid(reply.msglist(i).smsgid());
//  }

//  reply = client.OnOfflineMsg(request);
//  std::cout << "offlinemsgservice received: " << reply.msglist().size() << std::endl;

  client.OnGroupRelationNotify();

  cout << "client.OnGroupRelationNotify();" <<endl;
  //client.OnOnLoginOperationNotify();
  //client.OnFriendRelationNotify();
  client.OnGroupRelationNotify();
//  while(true)
//  { sleep(1);
//  }
   return 0;
}
