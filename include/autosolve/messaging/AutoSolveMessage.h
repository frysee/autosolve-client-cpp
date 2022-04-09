#pragma once

#include "IAutoSolveMessage.h"

#include <QMap>
#include <QJsonDocument>
#include <QJsonArray>

template <class K, class V>
struct QMapIteratorHelper
{
    const QMap<K, V> map;
    QMapIteratorHelper(const QMap<K, V> &map) : map(map) {}
    auto begin() { return map.keyValueBegin(); }
    auto end() { return map.keyValueEnd(); }
};

namespace AutoSolveClient
{
    class AutoSolveMessage : public Messaging::IAutoSolveMessage
    {
        // QString m_taskId;
        // QString m_apiKey;
        // qint64 m_createdAt;
        QString m_url;
        QString m_siteKey;
        QMap<QString, QString> m_renderParameters;
        int m_version;
        QString m_action;
        double m_minScore;
        QString m_proxy;
        bool m_proxyRequired{false};
        QString m_userAgent;
        QString m_cookies;

    public:
        // QString taskId() const { return m_taskId; }
        void setTaskId(const QString &taskId) { m_taskId = taskId; }

        // QString apiKey() const { return m_apiKey; }
        // void setApiKey(const QString &apiKey) { m_apiKey = apiKey; }

        // qint64 createdAt() const { return m_createdAt; }
        // void setCreatedAt(qint64 createdAt) { m_createdAt = cratedAt; }

        QString url() const { return m_url; };
        void setUrl(const QString &url) { m_url = url; };

        QString siteKey() const { return m_siteKey; };
        void setSiteKey(const QString &sitekey) { m_siteKey = sitekey; };

        QMap<QString, QString> renderParameters() const { return m_renderParameters; }
        void setRenderParameters(const QMap<QString, QString> &renderParams) { m_renderParameters = renderParams; }

        int version() const { return m_version; }
        void setVersion(int version) { m_version = version; }

        QString action() const { return m_action; }
        void setAction(const QString &action) { m_action = action; }

        double minScore() const { return m_minScore; }
        void setMinScore(double minScore) { m_minScore = minScore; }

        QString proxy() const { return m_proxy; }
        void setProxy(const QString &proxy) { m_proxy = proxy; }

        bool proxyRequired() const { return m_proxyRequired; }
        void setProxyRequired(bool proxyRequired) { m_proxyRequired = proxyRequired; }

        QString userAgent() const { return m_userAgent; }
        void setUserAgent(const QString &userAgent) { m_userAgent = userAgent; }

        QString cookies() const { return m_cookies; }
        void setCookies(const QString &cookies) { m_cookies = cookies; }

        QJsonObject toJson() const override
        {
            QJsonObject obj = IAutoSolveMessage::toJson();
            obj.insert("url", url());
            obj.insert("siteKey", siteKey());
            obj.insert("version", version());
            obj.insert("action", action());
            obj.insert("minScore", minScore());
            obj.insert("proxy", proxy());
            obj.insert("proxyRequired", proxyRequired());
            obj.insert("userAgent", userAgent());
            obj.insert("cookies", cookies());

            QJsonObject params;
            for (auto p : QMapIteratorHelper(renderParameters()))
                params.insert(p.first, p.second);
            obj.insert("renderParameters", params);

            return obj;
        }

        static AutoSolveMessage *fromJson(const QString &json)
        {
            auto doc = QJsonDocument::fromJson(json.toUtf8());
            if (doc.isNull())
                return nullptr;

            if (!doc.isObject())
                return nullptr;

            return AutoSolveMessage::fromJson(doc.object());
        }

        static AutoSolveMessage *fromJson(const QJsonObject &obj)
        {
            auto msg = new AutoSolveMessage;

            if (obj.contains("url"))
                msg->setUrl(obj.value("utl").toString());
            if (obj.contains("siteKey"))
                msg->setSiteKey(obj.value("siteKey").toString());
            if (obj.contains("version"))
                msg->setVersion(obj.value("version").toInt());
            if (obj.contains("action"))
                msg->setAction(obj.value("action").toString());
            if (obj.contains("minScore"))
                msg->setMinScore(obj.value("minScore").toDouble());
            if (obj.contains("proxy"))
                msg->setProxy(obj.value("proxy").toString());
            if (obj.contains("proxyRequired"))
                msg->setProxyRequired(obj.value("proxyRequired").toBool());
            if (obj.contains("userAgent"))
                msg->setUserAgent(obj.value("userAgent").toString());
            if (obj.contains("cookies"))
                msg->setCookies(obj.value("cookies").toString());
            if (obj.contains("renderParameters"))
            {
                auto renderParams = obj.value("renderParameters").toObject();
                QMap<QString, QString> rp;
                for (auto p : QMapIteratorHelper(renderParams.toVariantMap()))
                    rp.insert(p.first, p.second.toString());
                msg->setRenderParameters(rp);
            }

            return msg;
        }
    };
}
