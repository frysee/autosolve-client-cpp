#pragma once

#include "AutoSolveConnectResultType.h"
#include "AutoSolveConstants.h"
#include "AutoSolveGlobal.h"
#include "exceptions/AutoSolveExceptions.h"
#include "messaging/AutoSolveBacklogMessage.h"
#include "messaging/AutoSolveCancelMessage.h"
#include "messaging/AutoSolveCancelResponse.h"
#include "messaging/AutoSolveCancelResponseArray.h"
#include "messaging/AutoSolveConnectionEvent.h"
#include "messaging/AutoSolveMessage.h"
#include "messaging/AutoSolveResponse.h"

// QAMQP
#include <qamqpclient.h>
#include <qamqpexchange.h>
#include <qamqpqueue.h>

#include <QDateTime>
#include <QEventLoop>
#include <QJsonDocument>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

namespace AutoSolveClient {
  class AUTOSOLVE_EXPORT AutoSolve : public QObject {
    Q_OBJECT

    QString m_accessToken;
    QString m_clientKey;
    QString m_apiKey;
    bool m_debug;
    bool m_shouldAlertOnCancel;
    bool m_connected;

    QString m_accountId;
    QString m_receiverRoutingKey;
    QString m_cancelRoutingKey;
    QString m_tokenSendRoutingKey;
    QString m_cancelSendRoutingKey;
    QString m_receiverQueueName;
    QString m_directExchangeName;
    QString m_fanoutExchangeName;

    QList<Messaging::AutoSolveBacklogMessage *> m_backlog;
    QNetworkAccessManager m_httpClient;
    AutoSolveConstants m_autoSolveConstants;
    QString m_routingKey;
    QString m_sendRoutingKey;
    int m_recoveryAttempts;

    QAmqpClient m_client;
    QAmqpQueue *m_queue{nullptr};
    QAmqpExchange *m_channel{nullptr};
    QAmqpExchange *m_cancelChannel{nullptr};

    void setAccessToken(const QString &token) { m_accessToken = token; }
    void setClientKey(const QString &clientKey) { m_clientKey = clientKey; }
    void setApiKey(const QString &apiKey) { m_apiKey = apiKey; }
    void setDebug(bool debug) { m_debug = debug; }
    void setShouldAlertOnCancel(bool alert) { m_shouldAlertOnCancel = alert; }
    void setConnected(bool connected) { m_connected = connected; }

  public:
    AutoSolve(QObject *parent, const QString &accessToken, const QString &apiKey,
              const QString &clientKey, bool debug, bool shouldAlertOnCancel)
        : QObject(parent) {
      setAccessToken(accessToken);
      setClientKey(clientKey);
      setApiKey(apiKey);
      setDebug(debug);
      setShouldAlertOnCancel(shouldAlertOnCancel);

      connect(&m_client, &QAmqpClient::disconnected, this, &AutoSolve::onDisconnected);
    }

    AutoSolve(QObject *parent, const QString &clientKey, bool debug, bool shouldAlertOnCancel)
        : AutoSolve(parent, "", "", clientKey, debug, shouldAlertOnCancel) {}

    QString accessToken() const { return m_accessToken; }
    QString clientKey() const { return m_clientKey; }
    QString apiKey() const { return m_apiKey; }
    bool debug() const { return m_debug; }
    bool shouldAlertOnCancel() const { return m_shouldAlertOnCancel; }
    bool connected() const { return m_connected; }

    void onDisconnected() {
      try {
        emitLogEvent("Connection Shutdown.", m_autoSolveConstants.Info);
        //     emitLogEvent("Connection Shutdown: " + args.Initiator, m_autoSolveConstants.Info);
        if (m_connected) {
          m_connected = false;
          // if (args.Initiator != ShutdownInitiator.Application)
          // {
          //     emitLogEvent("Shutdown was not intended.", m_autoSolveConstants.Error);
          //     emit connectionEvent(Messaging::AutoSolveConnectionEvent::Shutdown);
          //     connectionLoop(false)
          // }
        }
      } catch (const std::exception &e) {
        emitLogEvent(e.what(), m_autoSolveConstants.Error);
      }
    }

    void init() {
      initAsync();

      // QEventLoop loop;
      // connect(this, &AutoSolve::connectionEvent, &loop, &QEventLoop::quit);
      // loop.exec();
    }

    void init(const QString &accessToken, const QString &apiKey) {
      setAccessToken(accessToken);
      setApiKey(apiKey);
      init();
    }

    void initAsync() {
      setRoutingObjects();
      connectionLoop(true);
    }

    void initAsync(const QString &accessToken, const QString &apiKey) {
      setAccessToken(accessToken);
      setApiKey(apiKey);
      initAsync();
    }

    bool sendTokenRequest(AutoSolveMessage *message) {
      return sendMessage(message, m_tokenSendRoutingKey, m_directExchangeName);
    }

    bool cancelTokenRequest(Messaging::AutoSolveCancelMessage *message) {
      if (shouldAlertOnCancel()) message->setResponseRequired(true);

      return sendMessage(message, m_cancelSendRoutingKey, m_fanoutExchangeName);
    }

    bool cancelAllRequests() {
      auto message = new Messaging::AutoSolveCancelMessage(apiKey());
      if (shouldAlertOnCancel()) message->setResponseRequired(true);

      return sendMessage(message, m_cancelSendRoutingKey, m_fanoutExchangeName);
    }

    bool closeConnection() {
      if (m_client.isConnected()) m_client.disconnectFromHost();

      return true;
    }

  signals:
    void logMessage(const QString &);
    void connectionEvent(Messaging::AutoSolveConnectionEvent);
    void responseReceived(AutoSolveResponse *);
    void cancelResponseReceived(AutoSolveCancelResponse *);

  private:
    bool beginConnection() { return bindQueue() && receiveMessages(); }

    void setRoutingObjects() {
      m_routingKey = apiKey().replace("-", "");
      m_sendRoutingKey = accessToken().replace("-", "");
      m_accountId = getUsernameFromAuthToken(accessToken());

      m_directExchangeName = buildWithAccountId(m_autoSolveConstants.DirectExchange);
      m_fanoutExchangeName = buildWithAccountId(m_autoSolveConstants.FanoutExchange);

      m_receiverRoutingKey = buildWithRoutingKey(m_autoSolveConstants.TokenReceiveRoute);
      m_cancelRoutingKey = buildWithRoutingKey(m_autoSolveConstants.TokenReceiveCancelRoute);
      m_tokenSendRoutingKey = buildWithAccessToken(m_autoSolveConstants.TokenSendRoute);
      m_cancelSendRoutingKey = buildWithAccessToken(m_autoSolveConstants.TokenCancelRoute);
      m_receiverQueueName = buildWithRoutingKey(m_autoSolveConstants.ReceiverQueueName);
    }

    AutoSolveConnectResultType connectionLoop(bool initialConnection) {
      bool keepGoing = true;
      while (keepGoing) {
        using namespace Exceptions;
        try {
          keepGoing = false;
          closeConnection();
          emitLogEvent("Validating credentials", m_autoSolveConstants.Info);
          validateInput();
          emitLogEvent("Creating connection", m_autoSolveConstants.Info);
          createConnection();
          emitLogEvent("Connection created", m_autoSolveConstants.Info);

          m_channel = m_client.createExchange(m_directExchangeName);
          emitLogEvent("Channel created", m_autoSolveConstants.Info);

          m_cancelChannel = m_client.createExchange(m_fanoutExchangeName);
          emitLogEvent("Cancel Channel created", m_autoSolveConstants.Info);

          m_connected = beginConnection();
          if (!m_connected) {
            throw new std::runtime_error("Connection failed");
          } else {
            emitLogEvent("Connection Successful", m_autoSolveConstants.Info);
            m_recoveryAttempts = 0;
            return AutoSolveConnectResultType::Success;
          }
        } catch (AutoSolveException &ex) {
          auto delay = getDelay(m_recoveryAttempts);
          m_connected = false;
          emitLogEvent(
              QString(
                  "Error in the connection process. AutoSolve could not verify the credentials. %1")
                  .arg(ex.what()),
              m_autoSolveConstants.Error);
          if (ex.type() == AutoSolveExceptionTypes::TooManyRequests) {
            delay = m_autoSolveConstants.RateLimitInterval;
          } else if (ex.type() != AutoSolveExceptionTypes::InitConnectionError) {
            if (initialConnection) {
              throw ex;
            } else {
              switch (ex.type()) {
                case AutoSolveExceptionTypes::InvalidApiKeyOrAccessToken:
                case AutoSolveExceptionTypes::InputValueError:
                  return AutoSolveConnectResultType::InvalidApiKeyOrAccessToken;
                case AutoSolveExceptionTypes::InvalidClientId:
                  return AutoSolveConnectResultType::InvalidClientId;
                default:
                  return AutoSolveConnectResultType::UnknownError;
              }
            }
          }
          break;

          QThread::sleep(delay);
          m_recoveryAttempts++;
          keepGoing = true;
        } catch (std::exception &ex) {
          (void)ex;
          auto delay = getDelay(m_recoveryAttempts);
          // switch (ex)
          //{
          //  case BrokerUnreachableException _:
          //      m_connected = false;
          //      emitLogEvent("Could not reach Rabbit server, waiting to retry",
          //      m_autoSolveConstants.Error); break;
          //  case OperationInterruptedException _:
          //      m_connected = false;
          //      emitLogEvent("Could not reach Rabbit server, waiting to retry",
          //      m_autoSolveConstants.Error); break;
          // default:
          m_connected = false;
          emitLogEvent("Unknown connection error occured. Re-attempting connection",
                       m_autoSolveConstants.Error);
          // break;
          // }

          QThread::sleep(delay);
          m_recoveryAttempts++;
          keepGoing = true;
        }
      }
      return AutoSolveConnectResultType::UnknownError;
    }

    bool bindQueue() {
      try {
        emitLogEvent("Binding queue to exchange", m_autoSolveConstants.Info);

        m_queue = m_client.createQueue(m_receiverQueueName, m_channel->channelNumber());
        m_queue->bind(m_channel, m_receiverRoutingKey);
        QEventLoop loop;
        QObject::connect(m_queue, &QAmqpQueue::bound, &loop, &QEventLoop::quit);
        loop.exec();

        m_queue->bind(m_channel, m_cancelRoutingKey);
        QObject::connect(m_queue, &QAmqpQueue::bound, &loop, &QEventLoop::quit);
        loop.exec();

        return true;
      } catch (const std::exception &e) {
        emitLogEvent(e.what(), m_autoSolveConstants.Error);
        emit connectionEvent(Messaging::AutoSolveConnectionEvent::InitError);
        using namespace AutoSolveClient::Exceptions;
        throw new AutoSolveException(AutoSolveExceptionTypes::InitConnectionError, e.what());
      }
    }

    bool sendMessage(Messaging::IAutoSolveMessage *message, const QString &route,
                     const QString &exchangeName) {
      if (connected()) {
        emitLogEvent("Sending message for taskId: " + message->taskId(), m_autoSolveConstants.Info);

        message->setCreatedAt(QDateTime::currentSecsSinceEpoch());
        message->setApiKey(apiKey());

        emitLogEvent(QString("Message Timestamp :: %1").arg(message->createdAt()),
                     m_autoSolveConstants.Info);

        auto messageJsonString
            = QString::fromUtf8(QJsonDocument(message->toJson()).toJson(QJsonDocument::Compact));

        emitLogEvent(messageJsonString, m_autoSolveConstants.Info);

        if (exchangeName == m_directExchangeName) {
          emitLogEvent("Sending Token Message", m_autoSolveConstants.Info);
          m_channel->publish(messageJsonString, route);
        } else {
          emitLogEvent("Sending Cancel Message", m_autoSolveConstants.Info);
          m_cancelChannel->publish(messageJsonString, route);
        }

        return true;
      } else {
        emitLogEvent(QString("Message send not possible for taskId: %1. Pushing to backlog")
                         .arg(message->taskId()),
                     m_autoSolveConstants.Info);
        m_backlog.append(new Messaging::AutoSolveBacklogMessage(message, route, exchangeName));
        return false;
      }
    }

    bool receiveMessages() {
      auto onReceived = [this]() {
        try {
          QAmqpMessage message = m_queue->dequeue();

          auto bodyStr = byteArrayToString(message.payload());
          if (message.routingKey() == m_receiverRoutingKey) {
            auto response = AutoSolveResponse::fromJson(bodyStr);
            if (response != nullptr) {
              emitLogEvent("Received message for taskId: " + response->taskId(),
                           m_autoSolveConstants.Info);
              emit responseReceived(response);
            }
          } else {
            auto response = Messaging::AutoSolveCancelResponseArray::fromJson(bodyStr);
            if (response != nullptr) {
              emitLogEvent(
                  QString("Received %1 cancelled requests").arg(response->requests().count()),
                  m_autoSolveConstants.Info);
              auto result = parseObjectsFromResponse(*response);
              emit cancelResponseReceived(result);
            }
          }
        } catch (const std::exception &ex) {
          emitLogEvent(ex.what(), m_autoSolveConstants.Error);
        }
      };

      auto onSuccess = [this]() {
        try {
          m_connected = true;
          emitLogEvent(QString("Consumer Registered: %1").arg(m_queue->consumerTag()),
                       m_autoSolveConstants.Info);
          emit connectionEvent(Messaging::AutoSolveConnectionEvent::Connected);
          handleMessageBacklog();
        } catch (const std::exception &ex) {
          emitLogEvent(ex.what(), m_autoSolveConstants.Error);
        }
      };

      connect(m_queue, &QAmqpQueue::messageReceived, onReceived);
      connect(m_queue, &QAmqpQueue::consuming, onSuccess);
      auto retCon = m_queue->consume(QAmqpQueue::coNoAck);

      QEventLoop loop;
      QObject::connect(m_queue, &QAmqpQueue::consuming, &loop, &QEventLoop::quit);
      loop.exec();

      return retCon;
    }

    void handleMessageBacklog() {
      if (m_backlog.count() > 0) {
        for (auto message : m_backlog) {
          auto msg = message->message();
          sendMessage(msg, message->route(), message->exchange());
        }

        m_backlog.clear();
      }
    }

    AutoSolveCancelResponse *parseObjectsFromResponse(
        const Messaging::AutoSolveCancelResponseArray &response) {
      auto result = new AutoSolveCancelResponse;
      for (auto jsObject : response.requests()) {
        auto responseMessage = AutoSolveMessage::fromJson(jsObject);
        result->requests().append(responseMessage);
      }

      return result;
    }

    void validateInput() {
      using namespace AutoSolveClient::Exceptions;
      try {
        auto uri = QUrl(QString("https://dash.autosolve.aycd.io/rest/%1/verify/%2?clientId=%3")
                            .arg(accessToken())
                            .arg(apiKey())
                            .arg(clientKey()));

        QNetworkRequest request(uri);
        auto response = m_httpClient.get(request);

        QEventLoop loop;
        QObject::connect(response, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        const auto status = response->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (status == 429) {
          throw new AutoSolveException(AutoSolveExceptionTypes::TooManyRequests);
        }

        switch (status) {
          case 200:
            return;

          case 400:
            throw new AutoSolveException(AutoSolveExceptionTypes::InvalidClientId);

          case 401:
            throw new AutoSolveException(AutoSolveExceptionTypes::InvalidApiKeyOrAccessToken);
        }
      } catch (const std::exception &ex) {
        emitLogEvent(ex.what(), m_autoSolveConstants.Error);
      }

      throw new AutoSolveException(AutoSolveExceptionTypes::Unknown);
    }

    void createConnection() {
      m_client.setUsername(m_accountId);
      m_client.setPassword(accessToken());
      m_client.setHost(m_autoSolveConstants.Hostname);
      m_client.setVirtualHost(m_autoSolveConstants.Vhost);
      m_client.setHeartbeatDelay(10);

      m_client.connectToHost();

      QEventLoop loop;
      QObject::connect(&m_client, &QAmqpClient::connected, &loop, &QEventLoop::quit);
      loop.exec();
    }

    QString getUsernameFromAuthToken(const QString &authToken) { return authToken.split('-')[0]; }

    QString buildWithRoutingKey(const QString &prefix) {
      return QString("%1.%2.%3").arg(prefix).arg(m_accountId).arg(m_routingKey);
    }

    QString buildWithAccessToken(const QString &prefix) {
      return QString("%1.%2").arg(prefix).arg(m_sendRoutingKey);
    }

    QString buildWithAccountId(const QString &prefix) {
      return QString("%1.%2").arg(prefix).arg(m_accountId);
    }

    QString byteArrayToString(const QByteArray &input) { return QString::fromLocal8Bit(input); }

    static inline const QList<long> Sequence{2000L, 3000L, 5000L, 8000L, 13000L, 21000L, 34000L};

    long getDelay(int attempts) {
      int index = attempts >= Sequence.count() ? Sequence.count() - 1 : attempts;
      return Sequence[index];
    }

    void emitLogEvent(const QString &message, const QString &type) {
      if (debug()) {
        emit logMessage(QString("%1 :: %2").arg(type).arg(message));
      }
    }
  };
}  // namespace AutoSolveClient