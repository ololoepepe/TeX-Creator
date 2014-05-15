#ifndef PARSER_H
#define PARSER_H

class QString;

#include "token.h"

#include <QList>

class Parser
{
public:
    explicit Parser(const QList<Token> &tokens = QList<Token>());
public:
    void setTokenList(const QList<Token> &tokens);
    QList<Token> tokenList() const;
    Token *parce(bool *ok = 0, QString *err = 0, Token *token = 0) const;
private:
    QList<Token> mtokens;
};

#endif // PARSER_H
