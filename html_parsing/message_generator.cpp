#include "message_generator.h"

MessageGenerator::MessageGenerator(QString& link, QObject* parent)
    : QObject(parent)
{
    m_parser = new HtmlParser(this);
}

QString MessageGenerator::generateMessage(QString htmlLink, QString exportLink)
{
    return HtmlParser::fetchNewsFeed(htmlLink, exportLink);
}
