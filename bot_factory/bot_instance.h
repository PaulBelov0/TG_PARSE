#ifndef BOT_INSTANCE_H
#define BOT_INSTANCE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QUrlQuery>
#include <QString>
#include <QTimer>
#include <QMap>
#include <QPointer>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <memory>

#include "html_parsing/message_generator.h"


class BotInstance : QObject
{
    Q_OBJECT
public:
    BotInstance(QString& botName, QJsonDocument& doc, QUrl& targetSite, QObject* parent = nullptr);

    void sendMessage();

    QString getBotName();
private:
    QString m_tgUrl;
    QString m_botToken;
    QString m_cfgFilename;
    QString m_botName;
    QString m_chatID;
    QUrl m_targetSite;

    std::shared_ptr<MessageGenerator> m_generator;
};

#endif // BOT_INSTANCE_H
