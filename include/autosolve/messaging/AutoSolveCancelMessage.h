#pragma once

#include "IAutoSolveMessage.h"

namespace AutoSolveClient::Messaging {
  class AutoSolveCancelMessage : public IAutoSolveMessage {
    bool m_responseRequired{false};

  public:
    AutoSolveCancelMessage(const QString &apiKey) : IAutoSolveMessage() { setApiKey(apiKey); }

    void setTaskId(const QString &taskId) { m_taskId = taskId; }

    bool responseRequired() const { return m_responseRequired; }
    void setResponseRequired(bool responseRequired) { m_responseRequired = responseRequired; }

    virtual QJsonObject toJson() const {
      auto obj = IAutoSolveMessage::toJson();
      obj.insert("responseRequired", responseRequired());
      return obj;
    }
  };
}  // namespace AutoSolveClient::Messaging
