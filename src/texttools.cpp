#include "texttools.h"

#include <BeQtGlobal>

#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QList>

#include <QDebug>

namespace TextTools
{

SearchResult::SearchResult(const QString *const t, int p, int l)
{
    txt = t;
    pos = p;
    len = l;
}

SearchResult::SearchResult(const SearchResult &other)
{
    *this = other;
}

//

QString SearchResult::text() const
{
    return txt ? txt->mid(pos, len) : QString();
}

int SearchResult::position() const
{
    return pos;
}

int SearchResult::length() const
{
    return len;
}

//

SearchResult &SearchResult::operator =(const SearchResult &other)
{
    txt = other.txt;
    pos = other.pos;
    len = other.len;
    return *this;
}

bool SearchResult::operator ==(const SearchResult &other) const
{
    return txt == other.txt && pos == other.pos && len == other.len;
}

//

QStringList removeDuplicates(const QStringList &list, Qt::CaseSensitivity cs, int *count)
{
    QStringList nlist = list;
    int c = removeDuplicates(&nlist, cs);
    if (count)
        *count = c;
    return nlist;
}

int removeDuplicates(QStringList *list, Qt::CaseSensitivity cs)
{
    if (!list)
        return 0;
    int count = 0;
    foreach (int i, bRangeR(list->size() - 1, 1))
    {
        foreach (int j, bRangeR(i - 1, 0))
        {
            if (!list->at(i).compare(list->at(j), cs))
            {
                list->removeAt(j);
                ++count;
            }
        }
    }
    return count;
}

QStringList sortComprising(const QStringList &list, Qt::CaseSensitivity cs)
{
    QStringList nlist = list;
    sortComprising(&nlist, cs);
    return nlist;
}

void sortComprising(QStringList *list, Qt::CaseSensitivity cs)
{
    if (!list || list->size() < 2)
        return;
    QStringList nlist;
    foreach (int i, bRangeR(list->size() - 1, 0))
    {
        foreach (int j, bRangeR(list->size() - 1, 0))
        {
            if (i != j && list->at(j).contains(list->at(i), cs))
            {
                QString s = list->takeAt(i);
                bool b = false;
                foreach (int k, bRangeD(0, nlist.size() - 1))
                {
                    if (s.contains(nlist.at(k), cs))
                    {
                        if (!s.compare(nlist.at(k), cs))
                            nlist.insert(k, s);
                        else
                            nlist.prepend(s);
                        b = true;
                        break;
                    }
                }
                if (!b)
                    nlist.append(s);
                break;
            }
        }
    }
    *list += nlist;
}

SearchResults match(const QString &text, const QRegExp &what, const QRegExp &prefixedBy, const QRegExp &postfixedBy)
{
    QList<SearchResult> list;
    if (text.isEmpty() || !what.isValid())
        return list;
    QStringList sl = text.split('\n');
    int coveredLength = 0;
    foreach (int i, bRangeD(0, sl.size() - 1))
    {
        const QString &line = sl.at(i);
        int pos = what.indexIn(line);
        while (pos >= 0)
        {
            int len = what.matchedLength();
            if (!prefixedBy.isEmpty() && prefixedBy.isValid())
            {
                int prind = prefixedBy.indexIn(line.mid(0, pos));
                if (prind < 0 || prind + prefixedBy.matchedLength() != pos)
                {
                    pos = what.indexIn(line, pos + len);
                    continue;
                }
            }
            if (!postfixedBy.isEmpty() && postfixedBy.isValid())
            {
                int poind = postfixedBy.indexIn(line.mid(pos + len));
                if (poind != 0)
                {
                    pos = what.indexIn(line, pos + len);
                    continue;
                }
            }
            list << SearchResult(&text, coveredLength + pos, len);
            pos = what.indexIn(line, pos + len);
        }
        coveredLength += line.length() + 1;
    }
    return list;
}

}
