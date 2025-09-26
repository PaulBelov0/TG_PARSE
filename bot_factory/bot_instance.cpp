#include "bot_instance.h"

BotInstance::BotInstance(QString& botName, QJsonDocument& doc, QUrl& targetSite, QString exportLink, QObject* parent)
    : QObject(parent)
    , m_previousMessage(new QString())
{
    m_botName = botName;
    m_targetSite = targetSite;
    m_tgUrl = "https://api.telegram.org/bot";
    m_chatID = QString("-1002848781330");
    m_exportLink = exportLink;

    QJsonObject jsonObj= doc.object();

    m_botToken = jsonObj["telegram_token"].toString();
    QString targetWeb = targetSite.toString();
    qDebug() << "URL TEST: " << targetWeb;

    m_generator = std::shared_ptr<MessageGenerator>(new MessageGenerator(targetWeb,  this));

    QTimer* timer = new QTimer(this);

    connect(timer, &QTimer::timeout, [this, targetWeb]{
        qDebug() <<  "sending from " << targetWeb;
        sendMessage(m_generator->generateMessage(targetWeb, m_exportLink));
    });
    timer->start(6000);

}

void BotInstance::sendMessage(QString message)
{
    if (message == *m_previousMessage)
        return;
    *m_previousMessage = message;
    QNetworkAccessManager* networkManager = new QNetworkAccessManager(this);

    QUrl url("https://api.telegram.org/bot" + m_botToken + "/sendMessage");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("parse_mode", "Markdown");
    params.addQueryItem("chat_id", m_chatID);
    params.addQueryItem("text", message);

    QNetworkReply *reply = networkManager->post(request, params.query(QUrl::FullyEncoded).toUtf8());


    QTimer::singleShot(1000, reply, &QNetworkReply::abort);

    qDebug() << "Connecting to:" << url.toString();
    qDebug() << "CONTENT: " << message.length();
    // qDebug() << "Request data:" << params.query(QUrl::FullyEncoded);

    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        qDebug() << "Reply finished! Error:" << reply->error();
        qDebug() << "Response:" << reply->readAll();
        reply->deleteLater();
    });

    QObject::connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError> &errors) {
        qWarning() << "SSL Errors, ignoring...";
        reply->ignoreSslErrors();
    });


    QObject::connect(reply, &QNetworkReply::errorOccurred, [](QNetworkReply::NetworkError error) {
        qWarning() << "Network error occurred:" << error;
    });
}
