// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: im.notify.proto

#include "im.notify.pb.h"
#include "im.notify.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/method_handler_impl.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace imnotify {

static const char* NotifyService_method_names[] = {
  "/imnotify.NotifyService/OnGroupRelation",
  "/imnotify.NotifyService/OnFriendRelation",
  "/imnotify.NotifyService/OnLoginOperation",
  "/imnotify.NotifyService/OnCommonMsgNotify",
};

std::unique_ptr< NotifyService::Stub> NotifyService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< NotifyService::Stub> stub(new NotifyService::Stub(channel));
  return stub;
}

NotifyService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_OnGroupRelation_(NotifyService_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_OnFriendRelation_(NotifyService_method_names[1], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_OnLoginOperation_(NotifyService_method_names[2], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_OnCommonMsgNotify_(NotifyService_method_names[3], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status NotifyService::Stub::OnGroupRelation(::grpc::ClientContext* context, const ::imnotify::GroupRelationNotify& request, ::imnotify::GroupRelationNotifyACK* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_OnGroupRelation_, context, request, response);
}

void NotifyService::Stub::experimental_async::OnGroupRelation(::grpc::ClientContext* context, const ::imnotify::GroupRelationNotify* request, ::imnotify::GroupRelationNotifyACK* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_OnGroupRelation_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::imnotify::GroupRelationNotifyACK>* NotifyService::Stub::AsyncOnGroupRelationRaw(::grpc::ClientContext* context, const ::imnotify::GroupRelationNotify& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::imnotify::GroupRelationNotifyACK>::Create(channel_.get(), cq, rpcmethod_OnGroupRelation_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::imnotify::GroupRelationNotifyACK>* NotifyService::Stub::PrepareAsyncOnGroupRelationRaw(::grpc::ClientContext* context, const ::imnotify::GroupRelationNotify& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::imnotify::GroupRelationNotifyACK>::Create(channel_.get(), cq, rpcmethod_OnGroupRelation_, context, request, false);
}

::grpc::Status NotifyService::Stub::OnFriendRelation(::grpc::ClientContext* context, const ::imnotify::FriendRelationNotify& request, ::imnotify::FriendRelationNotifyACK* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_OnFriendRelation_, context, request, response);
}

void NotifyService::Stub::experimental_async::OnFriendRelation(::grpc::ClientContext* context, const ::imnotify::FriendRelationNotify* request, ::imnotify::FriendRelationNotifyACK* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_OnFriendRelation_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::imnotify::FriendRelationNotifyACK>* NotifyService::Stub::AsyncOnFriendRelationRaw(::grpc::ClientContext* context, const ::imnotify::FriendRelationNotify& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::imnotify::FriendRelationNotifyACK>::Create(channel_.get(), cq, rpcmethod_OnFriendRelation_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::imnotify::FriendRelationNotifyACK>* NotifyService::Stub::PrepareAsyncOnFriendRelationRaw(::grpc::ClientContext* context, const ::imnotify::FriendRelationNotify& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::imnotify::FriendRelationNotifyACK>::Create(channel_.get(), cq, rpcmethod_OnFriendRelation_, context, request, false);
}

::grpc::Status NotifyService::Stub::OnLoginOperation(::grpc::ClientContext* context, const ::imnotify::LoginOperationNotify& request, ::imnotify::LoginOperationNotifyACK* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_OnLoginOperation_, context, request, response);
}

void NotifyService::Stub::experimental_async::OnLoginOperation(::grpc::ClientContext* context, const ::imnotify::LoginOperationNotify* request, ::imnotify::LoginOperationNotifyACK* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_OnLoginOperation_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::imnotify::LoginOperationNotifyACK>* NotifyService::Stub::AsyncOnLoginOperationRaw(::grpc::ClientContext* context, const ::imnotify::LoginOperationNotify& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::imnotify::LoginOperationNotifyACK>::Create(channel_.get(), cq, rpcmethod_OnLoginOperation_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::imnotify::LoginOperationNotifyACK>* NotifyService::Stub::PrepareAsyncOnLoginOperationRaw(::grpc::ClientContext* context, const ::imnotify::LoginOperationNotify& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::imnotify::LoginOperationNotifyACK>::Create(channel_.get(), cq, rpcmethod_OnLoginOperation_, context, request, false);
}

::grpc::Status NotifyService::Stub::OnCommonMsgNotify(::grpc::ClientContext* context, const ::imnotify::CommonMsgNotify& request, ::imnotify::CommonMsgNotifyACK* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_OnCommonMsgNotify_, context, request, response);
}

void NotifyService::Stub::experimental_async::OnCommonMsgNotify(::grpc::ClientContext* context, const ::imnotify::CommonMsgNotify* request, ::imnotify::CommonMsgNotifyACK* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_OnCommonMsgNotify_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::imnotify::CommonMsgNotifyACK>* NotifyService::Stub::AsyncOnCommonMsgNotifyRaw(::grpc::ClientContext* context, const ::imnotify::CommonMsgNotify& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::imnotify::CommonMsgNotifyACK>::Create(channel_.get(), cq, rpcmethod_OnCommonMsgNotify_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::imnotify::CommonMsgNotifyACK>* NotifyService::Stub::PrepareAsyncOnCommonMsgNotifyRaw(::grpc::ClientContext* context, const ::imnotify::CommonMsgNotify& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::imnotify::CommonMsgNotifyACK>::Create(channel_.get(), cq, rpcmethod_OnCommonMsgNotify_, context, request, false);
}

NotifyService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      NotifyService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< NotifyService::Service, ::imnotify::GroupRelationNotify, ::imnotify::GroupRelationNotifyACK>(
          std::mem_fn(&NotifyService::Service::OnGroupRelation), this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      NotifyService_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< NotifyService::Service, ::imnotify::FriendRelationNotify, ::imnotify::FriendRelationNotifyACK>(
          std::mem_fn(&NotifyService::Service::OnFriendRelation), this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      NotifyService_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< NotifyService::Service, ::imnotify::LoginOperationNotify, ::imnotify::LoginOperationNotifyACK>(
          std::mem_fn(&NotifyService::Service::OnLoginOperation), this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      NotifyService_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< NotifyService::Service, ::imnotify::CommonMsgNotify, ::imnotify::CommonMsgNotifyACK>(
          std::mem_fn(&NotifyService::Service::OnCommonMsgNotify), this)));
}

NotifyService::Service::~Service() {
}

::grpc::Status NotifyService::Service::OnGroupRelation(::grpc::ServerContext* context, const ::imnotify::GroupRelationNotify* request, ::imnotify::GroupRelationNotifyACK* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status NotifyService::Service::OnFriendRelation(::grpc::ServerContext* context, const ::imnotify::FriendRelationNotify* request, ::imnotify::FriendRelationNotifyACK* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status NotifyService::Service::OnLoginOperation(::grpc::ServerContext* context, const ::imnotify::LoginOperationNotify* request, ::imnotify::LoginOperationNotifyACK* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status NotifyService::Service::OnCommonMsgNotify(::grpc::ServerContext* context, const ::imnotify::CommonMsgNotify* request, ::imnotify::CommonMsgNotifyACK* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace imnotify

