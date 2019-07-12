// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: grpc.statistic.proto

#include "grpc.statistic.pb.h"
#include "grpc.statistic.grpc.pb.h"

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
namespace bi_analysis {

static const char* Statistic_method_names[] = {
  "/bi_analysis.Statistic/UpdateUserContext",
  "/bi_analysis.Statistic/PushUserAction",
};

std::unique_ptr< Statistic::Stub> Statistic::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< Statistic::Stub> stub(new Statistic::Stub(channel));
  return stub;
}

Statistic::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_UpdateUserContext_(Statistic_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_PushUserAction_(Statistic_method_names[1], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status Statistic::Stub::UpdateUserContext(::grpc::ClientContext* context, const ::bi_analysis::UserContext& request, ::bi_analysis::CommonRsp* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_UpdateUserContext_, context, request, response);
}

void Statistic::Stub::experimental_async::UpdateUserContext(::grpc::ClientContext* context, const ::bi_analysis::UserContext* request, ::bi_analysis::CommonRsp* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_UpdateUserContext_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::bi_analysis::CommonRsp>* Statistic::Stub::AsyncUpdateUserContextRaw(::grpc::ClientContext* context, const ::bi_analysis::UserContext& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::bi_analysis::CommonRsp>::Create(channel_.get(), cq, rpcmethod_UpdateUserContext_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::bi_analysis::CommonRsp>* Statistic::Stub::PrepareAsyncUpdateUserContextRaw(::grpc::ClientContext* context, const ::bi_analysis::UserContext& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::bi_analysis::CommonRsp>::Create(channel_.get(), cq, rpcmethod_UpdateUserContext_, context, request, false);
}

::grpc::Status Statistic::Stub::PushUserAction(::grpc::ClientContext* context, const ::bi_analysis::UserActions& request, ::bi_analysis::CommonRsp* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_PushUserAction_, context, request, response);
}

void Statistic::Stub::experimental_async::PushUserAction(::grpc::ClientContext* context, const ::bi_analysis::UserActions* request, ::bi_analysis::CommonRsp* response, std::function<void(::grpc::Status)> f) {
  return ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_PushUserAction_, context, request, response, std::move(f));
}

::grpc::ClientAsyncResponseReader< ::bi_analysis::CommonRsp>* Statistic::Stub::AsyncPushUserActionRaw(::grpc::ClientContext* context, const ::bi_analysis::UserActions& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::bi_analysis::CommonRsp>::Create(channel_.get(), cq, rpcmethod_PushUserAction_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::bi_analysis::CommonRsp>* Statistic::Stub::PrepareAsyncPushUserActionRaw(::grpc::ClientContext* context, const ::bi_analysis::UserActions& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::bi_analysis::CommonRsp>::Create(channel_.get(), cq, rpcmethod_PushUserAction_, context, request, false);
}

Statistic::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Statistic_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Statistic::Service, ::bi_analysis::UserContext, ::bi_analysis::CommonRsp>(
          std::mem_fn(&Statistic::Service::UpdateUserContext), this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Statistic_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Statistic::Service, ::bi_analysis::UserActions, ::bi_analysis::CommonRsp>(
          std::mem_fn(&Statistic::Service::PushUserAction), this)));
}

Statistic::Service::~Service() {
}

::grpc::Status Statistic::Service::UpdateUserContext(::grpc::ServerContext* context, const ::bi_analysis::UserContext* request, ::bi_analysis::CommonRsp* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Statistic::Service::PushUserAction(::grpc::ServerContext* context, const ::bi_analysis::UserActions* request, ::bi_analysis::CommonRsp* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace bi_analysis

