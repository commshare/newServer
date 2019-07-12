// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: im.radio.proto
#ifndef GRPC_im_2eradio_2eproto__INCLUDED
#define GRPC_im_2eradio_2eproto__INCLUDED

#include "im.radio.pb.h"

#include <functional>
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

namespace radionotify {

class RadioNotifyService final {
 public:
  static constexpr char const* service_full_name() {
    return "radionotify.RadioNotifyService";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status OnRadioMsgNotify(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify& request, ::radionotify::RadioMsgNotifyACK* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioMsgNotifyACK>> AsyncOnRadioMsgNotify(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioMsgNotifyACK>>(AsyncOnRadioMsgNotifyRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioMsgNotifyACK>> PrepareAsyncOnRadioMsgNotify(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioMsgNotifyACK>>(PrepareAsyncOnRadioMsgNotifyRaw(context, request, cq));
    }
    virtual ::grpc::Status OnRadioPushSetNotify(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify& request, ::radionotify::RadioPushSetNotifyACK* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioPushSetNotifyACK>> AsyncOnRadioPushSetNotify(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioPushSetNotifyACK>>(AsyncOnRadioPushSetNotifyRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioPushSetNotifyACK>> PrepareAsyncOnRadioPushSetNotify(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioPushSetNotifyACK>>(PrepareAsyncOnRadioPushSetNotifyRaw(context, request, cq));
    }
    class experimental_async_interface {
     public:
      virtual ~experimental_async_interface() {}
      virtual void OnRadioMsgNotify(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify* request, ::radionotify::RadioMsgNotifyACK* response, std::function<void(::grpc::Status)>) = 0;
      virtual void OnRadioPushSetNotify(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify* request, ::radionotify::RadioPushSetNotifyACK* response, std::function<void(::grpc::Status)>) = 0;
    };
    virtual class experimental_async_interface* experimental_async() { return nullptr; }
  private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioMsgNotifyACK>* AsyncOnRadioMsgNotifyRaw(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioMsgNotifyACK>* PrepareAsyncOnRadioMsgNotifyRaw(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioPushSetNotifyACK>* AsyncOnRadioPushSetNotifyRaw(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::radionotify::RadioPushSetNotifyACK>* PrepareAsyncOnRadioPushSetNotifyRaw(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);
    ::grpc::Status OnRadioMsgNotify(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify& request, ::radionotify::RadioMsgNotifyACK* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::radionotify::RadioMsgNotifyACK>> AsyncOnRadioMsgNotify(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::radionotify::RadioMsgNotifyACK>>(AsyncOnRadioMsgNotifyRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::radionotify::RadioMsgNotifyACK>> PrepareAsyncOnRadioMsgNotify(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::radionotify::RadioMsgNotifyACK>>(PrepareAsyncOnRadioMsgNotifyRaw(context, request, cq));
    }
    ::grpc::Status OnRadioPushSetNotify(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify& request, ::radionotify::RadioPushSetNotifyACK* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::radionotify::RadioPushSetNotifyACK>> AsyncOnRadioPushSetNotify(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::radionotify::RadioPushSetNotifyACK>>(AsyncOnRadioPushSetNotifyRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::radionotify::RadioPushSetNotifyACK>> PrepareAsyncOnRadioPushSetNotify(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::radionotify::RadioPushSetNotifyACK>>(PrepareAsyncOnRadioPushSetNotifyRaw(context, request, cq));
    }
    class experimental_async final :
      public StubInterface::experimental_async_interface {
     public:
      void OnRadioMsgNotify(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify* request, ::radionotify::RadioMsgNotifyACK* response, std::function<void(::grpc::Status)>) override;
      void OnRadioPushSetNotify(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify* request, ::radionotify::RadioPushSetNotifyACK* response, std::function<void(::grpc::Status)>) override;
     private:
      friend class Stub;
      explicit experimental_async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class experimental_async_interface* experimental_async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class experimental_async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::radionotify::RadioMsgNotifyACK>* AsyncOnRadioMsgNotifyRaw(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::radionotify::RadioMsgNotifyACK>* PrepareAsyncOnRadioMsgNotifyRaw(::grpc::ClientContext* context, const ::radionotify::RadioMsgNotify& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::radionotify::RadioPushSetNotifyACK>* AsyncOnRadioPushSetNotifyRaw(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::radionotify::RadioPushSetNotifyACK>* PrepareAsyncOnRadioPushSetNotifyRaw(::grpc::ClientContext* context, const ::radionotify::RadioPushSetNotify& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_OnRadioMsgNotify_;
    const ::grpc::internal::RpcMethod rpcmethod_OnRadioPushSetNotify_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status OnRadioMsgNotify(::grpc::ServerContext* context, const ::radionotify::RadioMsgNotify* request, ::radionotify::RadioMsgNotifyACK* response);
    virtual ::grpc::Status OnRadioPushSetNotify(::grpc::ServerContext* context, const ::radionotify::RadioPushSetNotify* request, ::radionotify::RadioPushSetNotifyACK* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_OnRadioMsgNotify : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_OnRadioMsgNotify() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_OnRadioMsgNotify() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status OnRadioMsgNotify(::grpc::ServerContext* context, const ::radionotify::RadioMsgNotify* request, ::radionotify::RadioMsgNotifyACK* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestOnRadioMsgNotify(::grpc::ServerContext* context, ::radionotify::RadioMsgNotify* request, ::grpc::ServerAsyncResponseWriter< ::radionotify::RadioMsgNotifyACK>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_OnRadioPushSetNotify : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_OnRadioPushSetNotify() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_OnRadioPushSetNotify() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status OnRadioPushSetNotify(::grpc::ServerContext* context, const ::radionotify::RadioPushSetNotify* request, ::radionotify::RadioPushSetNotifyACK* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestOnRadioPushSetNotify(::grpc::ServerContext* context, ::radionotify::RadioPushSetNotify* request, ::grpc::ServerAsyncResponseWriter< ::radionotify::RadioPushSetNotifyACK>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_OnRadioMsgNotify<WithAsyncMethod_OnRadioPushSetNotify<Service > > AsyncService;
  template <class BaseClass>
  class WithGenericMethod_OnRadioMsgNotify : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_OnRadioMsgNotify() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_OnRadioMsgNotify() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status OnRadioMsgNotify(::grpc::ServerContext* context, const ::radionotify::RadioMsgNotify* request, ::radionotify::RadioMsgNotifyACK* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_OnRadioPushSetNotify : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_OnRadioPushSetNotify() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_OnRadioPushSetNotify() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status OnRadioPushSetNotify(::grpc::ServerContext* context, const ::radionotify::RadioPushSetNotify* request, ::radionotify::RadioPushSetNotifyACK* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_OnRadioMsgNotify : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithRawMethod_OnRadioMsgNotify() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_OnRadioMsgNotify() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status OnRadioMsgNotify(::grpc::ServerContext* context, const ::radionotify::RadioMsgNotify* request, ::radionotify::RadioMsgNotifyACK* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestOnRadioMsgNotify(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_OnRadioPushSetNotify : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithRawMethod_OnRadioPushSetNotify() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_OnRadioPushSetNotify() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status OnRadioPushSetNotify(::grpc::ServerContext* context, const ::radionotify::RadioPushSetNotify* request, ::radionotify::RadioPushSetNotifyACK* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestOnRadioPushSetNotify(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_OnRadioMsgNotify : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithStreamedUnaryMethod_OnRadioMsgNotify() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler< ::radionotify::RadioMsgNotify, ::radionotify::RadioMsgNotifyACK>(std::bind(&WithStreamedUnaryMethod_OnRadioMsgNotify<BaseClass>::StreamedOnRadioMsgNotify, this, std::placeholders::_1, std::placeholders::_2)));
    }
    ~WithStreamedUnaryMethod_OnRadioMsgNotify() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status OnRadioMsgNotify(::grpc::ServerContext* context, const ::radionotify::RadioMsgNotify* request, ::radionotify::RadioMsgNotifyACK* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedOnRadioMsgNotify(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::radionotify::RadioMsgNotify,::radionotify::RadioMsgNotifyACK>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_OnRadioPushSetNotify : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithStreamedUnaryMethod_OnRadioPushSetNotify() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::StreamedUnaryHandler< ::radionotify::RadioPushSetNotify, ::radionotify::RadioPushSetNotifyACK>(std::bind(&WithStreamedUnaryMethod_OnRadioPushSetNotify<BaseClass>::StreamedOnRadioPushSetNotify, this, std::placeholders::_1, std::placeholders::_2)));
    }
    ~WithStreamedUnaryMethod_OnRadioPushSetNotify() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status OnRadioPushSetNotify(::grpc::ServerContext* context, const ::radionotify::RadioPushSetNotify* request, ::radionotify::RadioPushSetNotifyACK* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedOnRadioPushSetNotify(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::radionotify::RadioPushSetNotify,::radionotify::RadioPushSetNotifyACK>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_OnRadioMsgNotify<WithStreamedUnaryMethod_OnRadioPushSetNotify<Service > > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_OnRadioMsgNotify<WithStreamedUnaryMethod_OnRadioPushSetNotify<Service > > StreamedService;
};

}  // namespace radionotify


#endif  // GRPC_im_2eradio_2eproto__INCLUDED
