#include "html_parser.h"

HtmlParser::HtmlParser(QObject* parent)
    : QObject(parent)
{}

QString HtmlParser::fetchLatestNews(const QString &htmlLink)
{
    QString html = fetchHtml(htmlLink);
    if (html.isEmpty()) {
        return "Не удалось загрузить страницу";
    }
    return extractLatestNews(html, htmlLink);
}

QString HtmlParser::fetchNewsFeed(const QString &htmlLink, const QString& exportLink, int count)
{
    QString html = fetchHtml(htmlLink);
    if (html.isEmpty()) {
        return "Не удалось загрузить страницу";
    }

    QString data = fetchHtml(extractMultipleNews(html, htmlLink, exportLink, count));

    QString result = extractLatestNews(data , exportLink);

    if (result.length() > 2000) { // Оставляем запас
        int length = result.length();
        result = result.left(2000);
        result += "\n... и еще " + QString::number(length) + " новостей";
    }
    // qDebug() << "FETCHING: " << extractMultipleNews(html, htmlLink, exportLink, count);
    return result;
}

QString HtmlParser::fetchHtml(const QString &url) {
    QNetworkAccessManager manager;
    std::unique_ptr<QNetworkRequest> request = std::unique_ptr<QNetworkRequest>(new QNetworkRequest(QUrl(url)));

    request->setHeader(QNetworkRequest::UserAgentHeader,
                     "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:89.0) Gecko/20100101 Firefox/89.0");

    QNetworkReply *reply = manager.get(*request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Ошибка сети:" << reply->errorString() << "   " << url;
        reply->deleteLater();
        return QString();
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 300 && statusCode < 400) {
        QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (redirectUrl.isValid()) {
            if (redirectUrl.isRelative()) {
                redirectUrl = reply->url().resolved(redirectUrl);
            }
            reply->deleteLater();
            return fetchHtml(redirectUrl.toString());
        }
    }

    if (statusCode != 200) {
        qDebug() << "HTTP ошибка:" << statusCode;
        reply->deleteLater();
        return QString();
    }

    QString html = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    return html;
}

QString HtmlParser::escapeMarkdown(const QString &text) {
    QString escaped = text;
    escaped.replace("\\", "\\\\")
        .replace("_", "\\_")
        .replace("*", "\\*")
        .replace("`", "\\`")
        .replace("[", "\\[")
        .replace("]", "\\]");
    return escaped;
}

QString HtmlParser::decodeHtmlEntities(const QString &text) {
    QString decoded = text;
    decoded.replace("&nbsp;", " ")
        .replace("&amp;", "&")
        .replace("&quot;", "\"")
        .replace("&lt;", "<")
        .replace("&gt;", ">")
        .replace("&#160;", " ");
    return decoded;
}

QString HtmlParser::extractLatestNews(QString &html, const QString &link) {
    QString title = extractByHeadings(html);
    QString content = extractNewsContent(html, title);

    title = decodeHtmlEntities(title);
    content = decodeHtmlEntities(content);

    QString result;
    if (!title.isEmpty() && !content.isEmpty()) {
        result = "*" + escapeMarkdown(title) + "*" + "\n\n" + escapeMarkdown(content);
    } else if (!title.isEmpty()) {
        result = "*" + escapeMarkdown(title) + "*";
    } else if (!content.isEmpty()) {
        result = escapeMarkdown(content);
    } else {
        result = "Не удалось извлечь новость";
    }

    if (!link.isEmpty()) {
        result += "\n\n[📎 Источник](" + link + ")";
    }

    return result;
}

QString HtmlParser::extractMultipleNews(QString &html, const QString &baseUrl, const QString& exportLink, int count) {

    QStringList newsFeed = extractNewsFeed(html, baseUrl);

    if (newsFeed.isEmpty()) {
        return "Не удалось найти новостную ленту";
    }

    QString result;

    for (int i = 0; i < qMin(count, newsFeed.size()); ++i) {
        QStringList parts = newsFeed[i].split("|");
        if (parts.size() == 2) {
            QString url = parts[1];

            url = url.replace(baseUrl, exportLink);

            result = url;
        }
    }

    qDebug() << "RESULT LINK: " << result;
    return result;
}

QStringList HtmlParser::extractNewsFeed(QString &html, const QString &baseUrl) {
    QStringList newsItems;

    // Используем обычные строки вместо RAW
    QStringList containerPatterns;
    containerPatterns << "<div[^>]*class=\"[^\"]*news-item[^\"]*\"[^>]*>.*?<a[^>]*href=\"([^\"]*)\"[^>]*>.*?>(.*?)</a>.*?</div>";
    containerPatterns << "<article[^>]*>.*?<a[^>]*href=\"([^\"]*)\"[^>]*>.*?>(.*?)</a>.*?</article>";
    containerPatterns << "<div[^>]*class=\"[^\"]*news[^\"]*\"[^>]*>.*?<a[^>]*href=\"([^\"]*)\"[^>]*>.*?>(.*?)</a>.*?</div>";

    for (const QString &pattern : containerPatterns) {
        newsItems << extractNewsItemsByPattern(html, pattern, baseUrl);
        if (!newsItems.isEmpty()) break;
    }

    if (newsItems.isEmpty()) {
        newsItems << extractNewsItemsFromList(html, baseUrl);
    }

    return newsItems;
}

QStringList HtmlParser::extractNewsItemsFromList(QString &html, const QString &baseUrl) { //
    QStringList newsItems;

    QRegularExpression listRegex("<li[^>]*>.*?<a[^>]*href=\"([^\"]*)\"[^>]*>(.*?)</a>.*?</li>",
                                 QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
                                 );

    QRegularExpressionMatchIterator it = listRegex.globalMatch(html);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        if (match.capturedTexts().size() >= 3) {
            QString url = match.captured(1);
            QString title = cleanHtml(match.captured(2));

            if (url.startsWith("/")) {
                url = baseUrl + url;
            }

            if (!title.isEmpty() && title.length() > 10 && title.length() < 200) {
                newsItems << title + "|" + url;
            }
        }
    }

    return newsItems;
}

QStringList HtmlParser::extractNewsItemsByPattern(QString &html, const QString &pattern, const QString &baseUrl) {
    QStringList newsItems;

    QRegularExpression regex(pattern,
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatchIterator it = regex.globalMatch(html);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString url = match.captured(1);
        QString title = cleanHtml(match.captured(2));

        if (url.startsWith("/")) {
            url = baseUrl + url;
        }

        if (!title.isEmpty() && !url.isEmpty()) {
            newsItems << title + "|" + url;
        }
    }

    return newsItems;
}

QString HtmlParser::extractNewsContent(QString &html, const QString &title) {
    if (!title.isEmpty()) {
        QString contentNearTitle = extractContentNearTitle(html, title);
        if (!contentNearTitle.isEmpty()) return contentNearTitle;
    }

    QString articleContent = extractFromArticleTags(html);
    if (!articleContent.isEmpty()) return articleContent;

    QString classContent = extractByContentClasses(html);
    if (!classContent.isEmpty()) return classContent;

    QString timeContent = extractByTimePatterns(html);
    if (!timeContent.isEmpty()) return timeContent;

    return extractLongestTextBlock(html);
}

QString HtmlParser::extractContentNearTitle(QString &html, const QString &title) {
    QString escapedTitle = QRegularExpression::escape(cleanHtml(title));
    QRegularExpression regex(
        ">" + escapedTitle + "[^<]*<[^>]*>.*?<p[^>]*>(.*?)</p>",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatch match = regex.match(html);
    if (match.hasMatch()) {
        return cleanHtml(match.captured(1));
    }

    return QString();
}

QString HtmlParser::extractFromArticleTags(QString &html) {
    QRegularExpression regex(
        R"(<article[^>]*>.*?<p[^>]*>(.*?)</p>)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatch match = regex.match(html);
    if (match.hasMatch()) {
        return cleanHtml(match.captured(1));
    }

    return QString();
}

QString HtmlParser::extractByContentClasses(QString &html) {
    QStringList contentPatterns = {
        R"(<div[^>]*class="[^"]*content[^"]*"[^>]*>.*?<p[^>]*>(.*?)</p>)",
        R"(<div[^>]*class="[^"]*text[^"]*"[^>]*>.*?<p[^>]*>(.*?)</p>)",
        R"(<div[^>]*class="[^"]*article[^"]*text[^"]*"[^>]*>.*?<p[^>]*>(.*?)</p>)"
    };

    for (const QString &pattern : contentPatterns) {
        QRegularExpression regex(pattern,
                                 QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = regex.match(html);
        if (match.hasMatch()) {
            QString content = cleanHtml(match.captured(1));
            if (content.length() > 30) return content;
        }
    }

    return QString();
}

QString HtmlParser::extractBySemanticTags(QString &html) {
    QRegularExpression regex(
        R"(<article[^>]*>.*?<h[1-3][^>]*>(.*?)</h[1-3]>.*?<div[^>]*>(.*?)</div>.*?</article>)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatch match = regex.match(html);
    if (match.hasMatch()) {
        return match.captured(1) + ". " + match.captured(2);
    }

    return QString();
}

QString HtmlParser::extractByNewsClasses(QString &html) {
    QStringList newsPatterns = {
        R"(<div[^>]*class="[^"]*news[^"]*"[^>]*>.*?<h[1-3][^>]*>(.*?)</h[1-3]>)",
        R"(<div[^>]*class="[^"]*article[^"]*"[^>]*>.*?<h[1-3][^>]*>(.*?)</h[1-3]>)",
        R"(<div[^>]*class="[^"]*headline[^"]*"[^>]*>.*?<h[1-3][^>]*>(.*?)</h[1-3]>)"
    };

    for (const QString &pattern : newsPatterns) {
        QRegularExpression regex(pattern,
                                 QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = regex.match(html);
        if (match.hasMatch()) {
            return match.captured(1);
        }
    }

    return QString();
}

QString HtmlParser::extractByHeadings(QString &html) {
    QRegularExpression regex(
        R"(<h1[^>]*>(.*?)</h1>|<h2[^>]*>(.*?)</h2>|<h3[^>]*>(.*?)</h3>)",
        QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatchIterator it = regex.globalMatch(html);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString title;
        for (int i = 1; i <= 3; ++i) {
            if (!match.captured(i).isEmpty()) {
                title = match.captured(i);
                break;
            }
        }

        if (!title.isEmpty() &&
            !title.contains("menu", Qt::CaseInsensitive) &&
            !title.contains("nav", Qt::CaseInsensitive) &&
            !title.contains("footer", Qt::CaseInsensitive) &&
            title.length() > 10 && title.length() < 200) {
            return title;
        }
    }

    return QString();
}

QString HtmlParser::extractByTimePatterns(QString &html) {
    QRegularExpression regex(
        R"(<(time|span)[^>]*>(.*?)</\1>.*?<p[^>]*>(.*?)</p>)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatch match = regex.match(html);
    if (match.hasMatch()) {
        return match.captured(3);
    }

    return QString();
}

QString HtmlParser::extractLongestTextBlock(QString &html) {
    QRegularExpression regex(
        R"(<p[^>]*>(.*?)</p>)",
        QRegularExpression::CaseInsensitiveOption
    );

    QString longestText;
    int maxLength = 0;

    QRegularExpressionMatchIterator it = regex.globalMatch(html);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString text = cleanHtml(match.captured(1));

        if (text.length() > maxLength && text.length() > 50) {
            maxLength = text.length();
            longestText = text;
        }
    }

    return longestText;
}

QString HtmlParser::cleanHtml(const QString &html) {
    QTextDocument doc;
    doc.setHtml(html);
    return doc.toPlainText().simplified();
}
