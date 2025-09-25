#include "bot_factory.h"

BotFactory::BotFactory(QObject* parent)
{
    m_bots = std::unique_ptr<std::vector<BotInstance*>>(new std::vector<BotInstance*>());

    QString username = QString(std::getenv("USER"));

    qDebug() << "user:" << username;

    m_cfgDir = std::shared_ptr<QDir>(new QDir(":/bot_configs"));
    QStringList list = m_cfgDir->entryList(QStringList() << "*.json" << "*.JSON", QDir::Files);

    foreach (QString filename, list)
    {
        QFile file = QFile(":/bot_configs/" + filename);

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Не удалось открыть ресурс:" << ":/bot_configs/" + filename;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

        QUrl url;

        QJsonArray sources = doc.object()["news_sources"].toArray();
        for (const QJsonValue &source : sources)
            url = QUrl(source.toObject()["url"].toString());
        qDebug() << "URL:" << url.toString();

        m_bots->push_back(new BotInstance(filename, doc, url, this));
        BotInstance* bot = new BotInstance(filename, doc, url, this);
    }
}

BotFactory::~BotFactory()
{
    qDebug() << "Bot factory DESTRUCTED";
}
