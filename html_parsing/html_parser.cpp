#include "html_parser.h"

HtmlParser::HtmlParser(QObject* parent)
    : QObject(parent)
{}

QString& HtmlParser::fetchHtml(const QString &url)
{
    QString err("ERROR");;
    QNetworkAccessManager manager;
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");

    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error: " << reply->errorString();
        return err;
    }

    QString html = reply->readAll();
    reply->deleteLater();
    return html;
}

void HtmlParser::parseHtmlWithXmlReader(const QString &html)
{
    QXmlStreamReader reader(html);
    int depth = 0;

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType type = reader.readNext();

        switch (type) {
        case QXmlStreamReader::StartDocument:
            std::cout << "Document encoding: " << reader.documentEncoding().toString().toStdString() << std::endl;
            break;

        case QXmlStreamReader::StartElement: {
            QString tagName = reader.name().toString();
            std::cout << QString(depth * 2, ' ').toStdString()
                      << "<" << tagName.toStdString();

            // Вывод атрибутов
            for (const auto& attr : reader.attributes()) {
                std::cout << " " << attr.name().toString().toStdString()
                << "=\"" << attr.value().toString().toStdString() << "\"";
            }
            std::cout << ">" << std::endl;

            depth++;
            break;
        }

        case QXmlStreamReader::EndElement:
            depth--;
            std::cout << QString(depth * 2, ' ').toStdString()
                      << "</" << reader.name().toString().toStdString() << ">" << std::endl;
            break;

        case QXmlStreamReader::Characters: {
            QString text = reader.text().toString().trimmed();
            if (!text.isEmpty()) {
                std::cout << QString(depth * 2, ' ').toStdString()
                << "Text: " << text.toStdString() << std::endl;
            }
            break;
        }

        case QXmlStreamReader::Comment:
            std::cout << QString(depth * 2, ' ').toStdString()
                      << "<!-- " << reader.text().toString().toStdString() << " -->" << std::endl;
            break;

        case QXmlStreamReader::ProcessingInstruction:
            std::cout << "<?" << reader.processingInstructionTarget().toString().toStdString()
                      << " " << reader.processingInstructionData().toString().toStdString()
                      << "?>" << std::endl;
            break;

        case QXmlStreamReader::Invalid:
            std::cerr << "Invalid token: " << reader.errorString().toStdString() << std::endl;
            return;

        default:
            break;
        }
    }

    if (reader.hasError()) {
        std::cerr << "XML parsing error: " << reader.errorString().toStdString() << std::endl;
    }
}
