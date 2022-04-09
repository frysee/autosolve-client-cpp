#pragma once

#include <QJsonObject>
#include <QString>

namespace AutoSolveClient::Messaging {
  class IAutoSolveMessage {
  protected:
    QString m_taskId;
    qint64 m_createdAt;
    QString m_apiKey;

  public:
    QString taskId() const { return m_taskId; }

    qint64 createdAt() const { return m_createdAt; }
    void setCreatedAt(qint64 createdAt) { m_createdAt = createdAt; }

    QString apiKey() const { return m_apiKey; }
    void setApiKey(const QString &apiKey) { m_apiKey = apiKey; }

    // serialization
    virtual QJsonObject toJson() const {
      QJsonObject obj;
      obj.insert("taskId", taskId());
      obj.insert("createdAt", createdAt());
      obj.insert("apiKey", apiKey());
      return obj;
    }
  };
}  // namespace AutoSolveClient::Messaging