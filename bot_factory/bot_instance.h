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
#include <iostream>

#include "html_parsing/message_generator.h"


class BotInstance : QObject
{
    Q_OBJECT
public:
    BotInstance(QString& botName, QJsonDocument& doc, QUrl& targetSite, QString exportLink, QObject* parent = nullptr);

    void sendMessage(QString message);

    QString getBotName();
private:
    QString m_tgUrl;
    QString m_botToken;
    QString m_cfgFilename;
    QString m_botName;
    QString m_chatID;
    QString m_exportLink;
    QUrl m_targetSite;

    std::unique_ptr<QString> m_previousMessage;

    std::shared_ptr<MessageGenerator> m_generator;
};

#endif // BOT_INSTANCE_H
