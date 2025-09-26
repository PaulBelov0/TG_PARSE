#ifndef BOT_FACTORY_H
#define BOT_FACTORY_H

#include <QObject>
#include <QDir>
#include <QPointer>
#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QUrl>
#include <QApplication>
#include <QJsonArray>

#include <memory>

#include "bot_instance.h"

class BotFactory : QObject
{
    Q_OBJECT
public:
    explicit BotFactory(QObject* parent = nullptr);
    ~BotFactory();

private:
    std::shared_ptr<QDir> m_cfgDir;
    std::unique_ptr<std::vector<BotInstance*>> m_bots;
};

#endif // BOT_FACTORY_H
