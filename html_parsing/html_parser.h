#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#include <QObject>
#include <QXmlStreamReader>
#include <QEventLoop>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <iostream>

class HtmlParser : QObject
{
    Q_OBJECT
public:
    explicit HtmlParser(QObject* parent = nullptr);
    QString& fetchHtml(const QString &url);
    void parseHtmlWithXmlReader(const QString &html);
};

#endif // HTML_PARSER_H
