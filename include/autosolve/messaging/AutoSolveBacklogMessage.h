#pragma once

#include "IAutoSolveMessage.h"

namespace AutoSolveClient::Messaging {
  class AutoSolveBacklogMessage {
    IAutoSolveMessage *m_message{nullptr};
    QString m_route;
    QString m_exchange;

  public:
    AutoSolveBacklogMessage(IAutoSolveMessage *message, const QString &route,
                            const QString &exchange)
        : m_message(message), m_route(route), m_exchange(exchange) {}

    QString route() const { return m_route; }

    QString exchange() const { return m_exchange; }

    IAutoSolveMessage *message() const { return m_message; }
  };
}  // namespace AutoSolveClient::Messaging
