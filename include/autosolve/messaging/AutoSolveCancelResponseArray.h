#pragma once

#include <QList>

namespace AutoSolveClient::Messaging {
  class AutoSolveCancelResponseArray {
    QList<QString> m_requests;

  public:
    const QList<QString> &requests() const { return m_requests; }
    QList<QString> &requests() { return m_requests; }
    void setRequests(const QList<QString> &requests) { m_requests = requests; }

    static AutoSolveCancelResponseArray *fromJson(const QString &json) {
      auto doc = QJsonDocument::fromJson(json.toUtf8());
      if (doc.isNull()) return nullptr;

      if (!doc.isArray()) return nullptr;

      QStringList strings;
      auto arr = doc.array();
      for (auto elem : arr) strings.append(elem.toString());

      auto respArr = new AutoSolveCancelResponseArray;
      respArr->setRequests(strings);

      return respArr;
    }
  };
}  // namespace AutoSolveClient::Messaging
