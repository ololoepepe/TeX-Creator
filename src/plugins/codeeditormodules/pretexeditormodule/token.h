#ifndef TOKEN_H
#define TOKEN_H

class TokenData;

class QString;

/*============================================================================
================================ Token =======================================
============================================================================*/

class Token
{
public:
    enum Type
    {
        Unknown_Token = -1,
        BACKSLASH_Token = 0,
        FUNC_NAME_Token,
        SPEC_FUNC_NAME_Token,
        LBRACE_Token,
        RBRACE_Token,
        LBRACKET_Token,
        RBRACKET_Token,
        STRING_Token,
        INTEGER_Token,
        REAL_Token,
        SHARP_Token,
        EOF_Token,
        Program_Token,
        Function_Token,
        ObligArgList_Token,
        OptArgList_Token,
        OptArgListNE_Token,
        OblArg_Token,
        OptArg_Token,
        Subprogram_Token,
        Statement_Token,
        ArgumentNo_Token
    };
public:
    static QString typeToString(Type type, bool tokenWord = true);
public:
    explicit Token(Type type = Unknown_Token, int position = -1);
    Token(const Token &other);
    ~Token();
public:
    Type type() const;
    int position() const;
    TokenData *data() const;
    QString toString() const;
public:
    Token &operator= (const Token &other);
private:
    static TokenData *createData(Type type);
private:
    TokenData *mdata;
    int mpos;
};

#endif // TOKEN_H
