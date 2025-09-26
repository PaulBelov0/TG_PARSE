#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#include <QObject>
#include <QEventLoop>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QRegularExpression>
#include <QStringList>
#include <QString>
#include <QTextDocument>

class HtmlParser : public QObject
{
    Q_OBJECT
public:
    HtmlParser(QObject* parent = nullptr);

    static QString fetchLatestNews(const QString &htmlLink);
    static QString fetchNewsFeed(const QString &htmlLink, const QString& exportLink, int count = 1);

private:
    static QString fetchHtml(const QString &url);
    static QString extractLatestNews(QString &html, const QString &link);
    static QString extractMultipleNews(QString &html, const QString &baseUrl, const QString& exportLink, int count = 1);

    static QStringList extractNewsFeed(QString &html, const QString &baseUrl);
    static QStringList extractNewsItemsByPattern(QString &html, const QString &pattern, const QString &baseUrl);
    static QStringList extractNewsItemsFromList(QString &html, const QString &baseUrl);

    static QString extractBySemanticTags(QString &html);
    static QString extractByNewsClasses(QString &html);
    static QString extractByHeadings(QString &html);
    static QString extractByTimePatterns(QString &html);
    static QString extractLongestTextBlock(QString &html);

    static QString extractNewsContent(QString &html, const QString &title);
    static QString extractContentNearTitle(QString &html, const QString &title);
    static QString extractFromArticleTags(QString &html);
    static QString extractByContentClasses(QString &html);

    static QString cleanHtml(const QString &html);
    static QString escapeMarkdown(const QString &text);
    static QString decodeHtmlEntities(const QString &text);
};

#endif // HTML_PARSER_H
