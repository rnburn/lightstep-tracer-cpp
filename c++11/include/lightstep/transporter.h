#pragma once

#include <google/protobuf/message.h>
#include <opentracing/string_view.h>
#include <opentracing/util.h>

namespace lightstep {
// Transporter is the abstract base class for SyncTransporter and
// AsyncTransporter.
class Transporter {
 public:
  virtual ~Transporter() = default;
};

// SyncTransporter customizes how synchronous tracing reports are sent.
class SyncTransporter : public Transporter {
 public:
  // Synchronously sends `request` to a collector and sets `response` to the
  // collector's response.
  virtual opentracing::expected<void> Send(
      const google::protobuf::Message& request,
      google::protobuf::Message& response) = 0;
};

// AsyncTransporter customizes how asynchronous tracing reports are sent.
class AsyncTransporter : public Transporter {
 public:
  // Asynchronously sends `request` to a collector.
  //
  // On success, `response` is set to the collector's response and
  // `on_success(context)` is called.
  //
  // On failure, `on_failure(error, context)` is called.
  virtual void Send(const google::protobuf::Message& request,
                    google::protobuf::Message& response,
                    void (*on_success)(void* context),
                    void (*on_failure)(std::error_code error, void* context),
                    void* context) = 0;
};
}  // namespace lightstep
