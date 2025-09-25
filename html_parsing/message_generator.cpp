#include "message_generator.h"

MessageGenerator::MessageGenerator(QString& link, QObject* parent)
    : QObject(parent)
{
    m_msg = std::shared_ptr<QString>(new QString());
    m_link = std::shared_ptr<QString>(new QString(link));

    m_pasrser = new HtmlParser(this);
}

QString* MessageGenerator::generateMessage()
{
    return m_msg.get();
}
