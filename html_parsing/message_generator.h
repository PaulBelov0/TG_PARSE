#ifndef MESSAGE_GENERATOR_H
#define MESSAGE_GENERATOR_H

#include <QObject>
#include <QPointer>
#include <QString>

#include <memory>

#include "html_parser.h"

class MessageGenerator : QObject
{
    Q_OBJECT
public:
    MessageGenerator(QString& link, QObject* parent = nullptr);

    QString generateMessage(QString htmlLink, QString exportLink);
private:

    QPointer<HtmlParser> m_parser;
};

#endif // MESSAGE_GENERATOR_H
