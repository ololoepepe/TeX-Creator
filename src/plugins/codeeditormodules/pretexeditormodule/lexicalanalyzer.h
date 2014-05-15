#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H

class QChar;
class QTextCodec;

#include "token.h"

#include <QString>
#include <QList>
#include <QStack>
#include <QChar>

class LexicalAnalyzer
{
public:
    explicit LexicalAnalyzer(const QString &source, const QString &fileName = QString(), QTextCodec *codec = 0);
public:
    void setSource(const QString source);
    void setFileName(const QString &fileName);
    void setCodec(QTextCodec *codec);
    void setCodecName(const QString &codecName);
    QString source() const;
    QString fileName() const;
    QTextCodec *codec() const;
    QString codecName() const;
    QList<Token> analyze(bool *ok = 0, QString *err = 0, int *pos = 0, QString *fn = 0);
private:
    enum CharType
    {
        NoChar = 0,
        BackslashChar,
        LeftBraceChar,
        RightBraceChar,
        LeftBracketChar,
        RightBracketChar,
        PercentChar,
        NChar,
        TChar,
        FuncNameChar,
        OtherChar
    };
private:
    typedef bool (*MatchFunction)(const QString &, int &);
    typedef Token (*ParceFunction)(const QString &, int);
private:
    class Rule
    {
    public:
        explicit Rule();
        explicit Rule(MatchFunction mf, ParceFunction pf);
    public:
        bool match(const QString &s, int &matchedLength) const;
        Token parce(const QString &s, int pos = -1) const;
    public:
        MatchFunction matchFunction;
        ParceFunction parceFunction;
    };
private:
    static QList<Token> analyze(QString s, const QString &fileName, QTextCodec *codec, bool *ok = 0, QString *err = 0,
                                int *pos = 0, QString *fn = 0);
    static bool removeNonterminals(QString &s, int &matchedLength, QString *err = 0, int *pos = 0);
private:
    QString msource;
    QString mfileName;
    QTextCodec *mcodec;
};

#endif // LEXICALANALYZER_H
