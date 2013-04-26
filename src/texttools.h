#ifndef TEXTTOOLS_H
#define TEXTTOOLS_H

#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QList>

namespace TextTools
{

class SearchResult
{
public:
    explicit SearchResult(const QString *const t, int p, int l);
    SearchResult(const SearchResult &other);
public:
    QString text() const;
    int position() const;
    int length() const;
public:
    SearchResult &operator =(const SearchResult &other);
    bool operator ==(const SearchResult &other) const;
private:
    const QString *txt;
    int pos;
    int len;
};

typedef QList<SearchResult> SearchResults;

QStringList removeDuplicates(const QStringList &list, Qt::CaseSensitivity cs = Qt::CaseSensitive, int *count  = 0);
int removeDuplicates(QStringList *list, Qt::CaseSensitivity cs = Qt::CaseSensitive);
QStringList sortComprising(const QStringList &list, Qt::CaseSensitivity cs = Qt::CaseSensitive);
void sortComprising(QStringList *list, Qt::CaseSensitivity cs = Qt::CaseSensitive);
SearchResults match(const QString &text, const QRegExp &what, const QRegExp &prefixedBy = QRegExp(),
                    const QRegExp &postfixedBy = QRegExp());

}

#endif // TEXTTOOLS_H
