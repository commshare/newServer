// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: grpc.statistic.proto
#ifndef GRPC_grpc_2estatistic_2eproto__INCLUDED
#define GRPC_grpc_2estatistic_2eproto__INCLUDED

#include "grpc.statistic.pb.h"

#include <grpcpp/impl/codegen/async_generic_service.h>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/method_handler_impl.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/codegen/rpc_method.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/impl/codegen/stub_options.h>
#include <grpcpp/impl/codegen/sync_stream.h>

namespace grpc {
class CompletionQueue;
class Channel;
class ServerCompletionQueue;
class ServerContext;
}  // namespace grpc

namespace grpc {

// 统计服务接口
class Statistic final {
 public:
  static constexpr char const* service_full_name() {
    return "grpc.Statistic";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status UpdateUserContext(::grpc::ClientContext* context, const ::grpc::UserContexts& request, ::grpc::CommonRsp* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>> AsyncUpdateUserContext(::grpc::ClientContext* context, const ::grpc::UserContexts& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>>(AsyncUpdateUserContextRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>> PrepareAsyncUpdateUserContext(::grpc::ClientContext* context, const ::grpc::UserContexts& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>>(PrepareAsyncUpdateUserContextRaw(context, request, cq));
    }
    virtual ::grpc::Status PushUserAction(::grpc::ClientContext* context, const ::grpc::UserActions& request, ::grpc::CommonRsp* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>> AsyncPushUserAction(::grpc::ClientContext* context, const ::grpc::UserActions& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>>(AsyncPushUserActionRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>> PrepareAsyncPushUserAction(::grpc::ClientContext* context, const ::grpc::UserActions& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>>(PrepareAsyncPushUserActionRaw(context, request, cq));
    }
  private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>* AsyncUpdateUserContextRaw(::grpc::ClientContext* context, const ::grpc::UserContexts& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>* PrepareAsyncUpdateUserContextRaw(::grpc::ClientContext* context, const ::grpc::UserContexts& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>* AsyncPushUserActionRaw(::grpc::ClientContext* context, const ::grpc::UserActions& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::grpc::CommonRsp>* PrepareAsyncPushUserActionRaw(::grpc::ClientContext* context, const ::grpc::UserActions& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);
    ::grpc::Status UpdateUserContext(::grpc::ClientContext* context, const ::grpc::UserContexts& request, ::grpc::CommonRsp* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>> AsyncUpdateUserContext(::grpc::ClientContext* context, const ::grpc::UserContexts& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>>(AsyncUpdateUserContextRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>> PrepareAsyncUpdateUserContext(::grpc::ClientContext* context, const ::grpc::UserContexts& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>>(PrepareAsyncUpdateUserContextRaw(context, request, cq));
    }
    ::grpc::Status PushUserAction(::grpc::ClientContext* context, const ::grpc::UserActions& request, ::grpc::CommonRsp* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>> AsyncPushUserAction(::grpc::ClientContext* context, const ::grpc::UserActions& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>>(AsyncPushUserActionRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>> PrepareAsyncPushUserAction(::grpc::ClientContext* context, const ::grpc::UserActions& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>>(PrepareAsyncPushUserActionRaw(context, request, cq));
    }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>* AsyncUpdateUserContextRaw(::grpc::ClientContext* context, const ::grpc::UserContexts& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>* PrepareAsyncUpdateUserContextRaw(::grpc::ClientContext* context, const ::grpc::UserContexts& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>* AsyncPushUserActionRaw(::grpc::ClientContext* context, const ::grpc::UserActions& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::grpc::CommonRsp>* PrepareAsyncPushUserActionRaw(::grpc::ClientContext* context, const ::grpc::UserActions& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_UpdateUserContext_;
    const ::grpc::internal::RpcMethod rpcmethod_PushUserAction_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status UpdateUserContext(::grpc::ServerContext* context, const ::grpc::UserContexts* request, ::grpc::CommonRsp* response);
    virtual ::grpc::Status PushUserAction(::grpc::ServerContext* context, const ::grpc::UserActions* request, ::grpc::CommonRsp* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_UpdateUserContext : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_UpdateUserContext() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_UpdateUserContext() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status UpdateUserContext(::grpc::ServerContext* context, const ::grpc::UserContexts* request, ::grpc::CommonRsp* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestUpdateUserContext(::grpc::ServerContext* context, ::grpc::UserContexts* request, ::grpc::ServerAsyncResponseWriter< ::grpc::CommonRsp>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_PushUserAction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_PushUserAction() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_PushUserAction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status PushUserAction(::grpc::ServerContext* context, const ::grpc::UserActions* request, ::grpc::CommonRsp* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestPushUserAction(::grpc::ServerContext* context, ::grpc::UserActions* request, ::grpc::ServerAsyncResponseWriter< ::grpc::CommonRsp>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_UpdateUserContext<WithAsyncMethod_PushUserAction<Service > > AsyncService;
  template <class BaseClass>
  class WithGenericMethod_UpdateUserContext : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_UpdateUserContext() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_UpdateUserContext() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status UpdateUserContext(::grpc::ServerContext* context, const ::grpc::UserContexts* request, ::grpc::CommonRsp* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_PushUserAction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_PushUserAction() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_PushUserAction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status PushUserAction(::grpc::ServerContext* context, const ::grpc::UserActions* request, ::grpc::CommonRsp* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_UpdateUserContext : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithRawMethod_UpdateUserContext() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_UpdateUserContext() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status UpdateUserContext(::grpc::ServerContext* context, const ::grpc::UserContexts* request, ::grpc::CommonRsp* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestUpdateUserContext(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_PushUserAction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithRawMethod_PushUserAction() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_PushUserAction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status PushUserAction(::grpc::ServerContext* context, const ::grpc::UserActions* request, ::grpc::CommonRsp* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestPushUserAction(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_UpdateUserContext : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithStreamedUnaryMethod_UpdateUserContext() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler< ::grpc::UserContexts, ::grpc::CommonRsp>(std::bind(&WithStreamedUnaryMethod_UpdateUserContext<BaseClass>::StreamedUpdateUserContext, this, std::placeholders::_1, std::placeholders::_2)));
    }
    ~WithStreamedUnaryMethod_UpdateUserContext() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status UpdateUserContext(::grpc::ServerContext* context, const ::grpc::UserContexts* request, ::grpc::CommonRsp* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedUpdateUserContext(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::grpc::UserContexts,::grpc::CommonRsp>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_PushUserAction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithStreamedUnaryMethod_PushUserAction() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::StreamedUnaryHandler< ::grpc::UserActions, ::grpc::CommonRsp>(std::bind(&WithStreamedUnaryMethod_PushUserAction<BaseClass>::StreamedPushUserAction, this, std::placeholders::_1, std::placeholders::_2)));
    }
    ~WithStreamedUnaryMethod_PushUserAction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status PushUserAction(::grpc::ServerContext* context, const ::grpc::UserActions* request, ::grpc::CommonRsp* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedPushUserAction(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::grpc::UserActions,::grpc::CommonRsp>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_UpdateUserContext<WithStreamedUnaryMethod_PushUserAction<Service > > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_UpdateUserContext<WithStreamedUnaryMethod_PushUserAction<Service > > StreamedService;
};

}  // namespace grpc


#endif  // GRPC_grpc_2estatistic_2eproto__INCLUDED
