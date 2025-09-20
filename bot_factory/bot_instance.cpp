#include "bot_instance.h"

BotInstance::BotInstance(QString& botName, QJsonDocument& doc, QUrl& targetSite, QObject* parent)
    : QObject(parent)
{
    m_botName = botName;
    m_targetSite = targetSite;
    m_tgUrl = "https://api.telegram.org/bot";
    m_chatID = QString("-1002848781330");
    QJsonObject jsonObj= doc.object();

    m_botToken = jsonObj["telegram_token"].toString();

    qDebug() << m_botToken;
    QTimer* timer = new QTimer(this);

    connect(timer, &QTimer::timeout, [this]{
        qDebug() <<  "send";
        sendMessage();
    });
    timer->start(5000);

}

void BotInstance::sendMessage()
{
    QNetworkAccessManager* networkManager = new QNetworkAccessManager(this);

    QUrl url("https://api.telegram.org/bot" + m_botToken + "/sendMessage");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("chat_id", m_chatID);
    params.addQueryItem("text", "Test Message");

    QNetworkReply *reply = networkManager->post(request, params.query(QUrl::FullyEncoded).toUtf8());

    // Таймаут 30 секунд
    QTimer::singleShot(6000, reply, &QNetworkReply::abort);

    // DEBUG: Проверяем соединение
    qDebug() << "Connecting to:" << url.toString();
    qDebug() << "Request data:" << params.query(QUrl::FullyEncoded);

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
