#pragma once

#include <QJsonDocument>
#include <QString>

#include "AutoSolveMessage.h"

namespace AutoSolveClient {
  class AutoSolveResponse {
    AutoSolveMessage *m_message{nullptr};
    qint64 m_createdAt;
    QString m_token;
    QString m_taskId;

  public:
    AutoSolveMessage *request() const { return m_message; }
    void setRequest(AutoSolveMessage *message) { m_message = message; }

    qint64 createdAt() const { return m_createdAt; }
    void setCreatedAt(qint64 createdAt) { m_createdAt = createdAt; }

    QString token() const { return m_token; }
    void setToken(const QString &token) { m_token = token; };

    QString taskId() const { return m_taskId; }
    void setTaskId(const QString &taskId) { m_taskId = taskId; }

    static AutoSolveResponse *fromJson(const QString &json) {
      auto doc = QJsonDocument::fromJson(json.toUtf8());
      if (doc.isNull()) return nullptr;

      if (!doc.isObject()) return nullptr;

      auto resp = new AutoSolveResponse;

      auto obj = doc.object();
      if (obj.contains("createdAt")) resp->setCreatedAt(obj.value("createdAt").toInteger());
      if (obj.contains("token")) resp->setToken(obj.value("token").toString());
      if (obj.contains("taskId")) resp->setTaskId(obj.value("taskId").toString());

      // deserialize request
      if (obj.contains("request")) {
        auto req = AutoSolveMessage::fromJson(obj.value("request").toObject());
        resp->setRequest(req);
      }

      return resp;
    }
  };
}  // namespace AutoSolveClient
