#pragma once

#include <QString>

namespace AutoSolveClient {
  class AutoSolveConstants {
  public:
    const QString Hostname = "amqp.autosolve.aycd.io";
    const QString Vhost = "oneclick";

    // NAMES
    const QString DirectExchange = "exchanges.direct";
    const QString FanoutExchange = "exchanges.fanout";
    const QString ReceiverQueueName = "queues.response.direct";

    // ROUTES
    const QString TokenSendRoute = "routes.request.token";
    const QString TokenReceiveRoute = "routes.response.token";
    const QString TokenCancelRoute = "routes.request.token.cancel";
    const QString TokenReceiveCancelRoute = "routes.response.token.cancel";
    const bool AutoAck = true;

    // ERROR
    const QString Error = "ERROR";
    const QString Info = "INFO";
    const int RateLimitInterval = 60000;
  };
}  // namespace AutoSolveClient
