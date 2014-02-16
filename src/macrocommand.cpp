#include "macrocommand.h"
#include "macrocommandargument.h"
#include "global.h"
#include "macroexecutionstack.h"
#include "application.h"

#include <BAbstractCodeEditorDocument>
#include <BeQt>
#include <BTextTools>

#include <QString>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QChar>
#include <QList>
#include <QMap>
#include <QRegExp>
#include <QCoreApplication>
#include <QPlainTextEdit>
#include <QThread>
#include <QTextDocument>
#include <QFileInfo>
#include <QProcess>
#include <QByteArray>
#include <QTextCodec>
#include <QTextStream>

#include <cmath>

#include <QDebug>

#define CONVERT(textVar, suffix) \
bool ok##suffix = false; \
double d##suffix = 0.0; \
int i##suffix = 0; \
bool b##suffix = false; \
if (textVar.contains('.') || "inf" == textVar) \
{ \
    d##suffix = textVar.toDouble(&ok##suffix); \
    if (!ok##suffix) \
        return "Unable to convert"; \
    b##suffix = true; \
} \
else \
{ \
    i##suffix = textVar.toInt(&ok##suffix); \
    if (!ok##suffix) \
        return "Unable to convert"; \
}

#define CONVERT2(textVar, suffix) \
bool ok##suffix = false; \
double d##suffix = 0.0; \
int i##suffix = 0; \
bool b##suffix = false; \
if (textVar.contains('.') || "inf" == textVar) \
{ \
    d##suffix = textVar.toDouble(&ok##suffix); \
    if (!ok##suffix) \
      return bRet(error, QString("Unable to convert"), QString("false")); \
    b##suffix = true; \
} \
else \
{ \
    i##suffix = textVar.toInt(&ok##suffix); \
    if (!ok##suffix) \
        return bRet(error, QString("Unable to convert"), QString("false")); \
}

#define CONVERT3(textVar, suffix) \
bool okd##suffix = false; \
bool oki##suffix = false; \
double d##suffix = 0.0; \
int i##suffix = 0; \
if (textVar.contains('.')) \
    d##suffix = textVar.toDouble(&okd##suffix); \
else \
    i##suffix = textVar.toDouble(&oki##suffix); \

#define CONVERT4(textVar, suffix) \
bool okb##suffix = false; \
bool b##suffix = false; \
if (textVar.contains('.')) \
{ \
    double d = textVar.toDouble(&okb##suffix); \
    if (okb##suffix) \
        b##suffix = (bool) d; \
} \
else if ("true" == textVar.toLower()) \
{ \
    b##suffix = true; \
    okb##suffix = true; \
} \
else if ("false" == textVar.toLower()) \
{ \
    okb##suffix = true; \
} \
else \
{ \
    int i = textVar.toDouble(&okb##suffix); \
    if (okb##suffix) \
        b##suffix = (bool) i; \
} \
if (!okb##suffix) \
    return "Failed to convert";

template<typename T> T ctan(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::tan(t);
}

template<typename T> T sec(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::sin(t);
}

template<typename T> T csc(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::cos(t);
}

template<typename T> T acot(T t)
{
    static const T t0 = (T) 0;
    static const T t1 = (T) 1;
    static const T t2 = (T) 2;
    static const T t4 = (T) 4;
    if (t >= t0)
        return std::asin(t1 / std::sqrt(t1 + std::pow(t, t2)));
    else
        return std::atan(t1) * t4 - std::asin(t1 / std::sqrt(t1 + std::pow(t, t2)));
}

template<typename T> T asec(T t)
{
    static const T t1 = (T) 1;
    return std::acos(t1 / t);
}

template<typename T> T acsc(T t)
{
    static const T t1 = (T) 1;
    return std::asin(t1 / t);
}

template<typename T> T cth(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::tanh(t);
}

template<typename T> T sech(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::sinh(t);
}

template<typename T> T csch(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::cosh(t);
}

template<typename T> T arsh(T t)
{
    static const T t1 = (T) 1;
    static const T t2 = (T) 2;
    return std::log(t + std::sqrt(std::pow(t, t2) + t1));
}

template<typename T> T arch(T t)
{
    static const T t1 = (T) 1;
    static const T t2 = (T) 2;
    return std::log(t + std::sqrt(std::pow(t, t2) - t1));
}

template<typename T> T arth(T t)
{
    static const T t1 = (T) 1;
    return 0.5 * (double) std::log((t1 + t) / (t1 - t));
}

template<typename T> T arcth(T t)
{
    static const T t1 = (T) 1;
    return 0.5 * (double) std::log((t + t1) / (t - t1));
}

template<typename T> T arsch(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::sinh(t);
}

template<typename T> T arcsch(T t)
{
    static const T t1 = (T) 1;
    return t1 / std::cosh(t);
}

template<typename T> T anyLog(T base, T t)
{
    return std::log(t) / std::log(base);
}

template<typename T> T root(T base, T p)
{
    static const T t1 = (T) 1;
    return std::pow(base, t1 / p);
}

/*============================================================================
================================ ThreadHack ==================================
============================================================================*/

class ThreadHack : public QThread
{
public:
    static void waitBlocking(int msecs);
};

/*============================================================================
================================ FunctionInfo ================================
============================================================================*/

struct FunctionInfo
{
public:
    typedef AbstractMacroCommand *(*Function)(const QList<MacroCommandArgument> &);
public:
    Function func;
    int mndArgCount;
    int optArgCount;
public:
    explicit FunctionInfo();
    explicit FunctionInfo(Function f, int mnd = 0, int opt = 0);
public:
    bool isValid() const;
};

/*============================================================================
================================ MultiArgMacroCommand ========================
============================================================================*/

class MultiArgMacroCommand : public AbstractMacroCommand
{
public:
    ~MultiArgMacroCommand();
protected:
    explicit MultiArgMacroCommand(const QList<MacroCommandArgument> &args);
public:
    void clear();
    bool isValid() const;
protected:
    bool compareInternal(const AbstractMacroCommand *other) const;
protected:
    QList<MacroCommandArgument> margs;
};

/*============================================================================
================================ InsertMacroCommand ==========================
============================================================================*/

class InsertMacroCommand : public AbstractMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
public:
    explicit InsertMacroCommand(const QString &txt);
    ~InsertMacroCommand();
private:
    explicit InsertMacroCommand(const MacroCommandArgument &a);

public:
    void clear();
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    bool isValid() const;
    AbstractMacroCommand *clone() const;
    bool append(const QString &txt);
protected:
    bool compareInternal(const AbstractMacroCommand *other) const;
private:
    MacroCommandArgument arg;
};

/*============================================================================
================================ PressMacroCommand ===========================
============================================================================*/

class PressMacroCommand : public AbstractMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
public:
    explicit PressMacroCommand(int k, Qt::KeyboardModifiers m);
    ~PressMacroCommand();
private:
    explicit PressMacroCommand(const MacroCommandArgument &a);
public:
    void clear();
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    bool isValid() const;
    AbstractMacroCommand *clone() const;
protected:
    bool compareInternal(const AbstractMacroCommand *other) const;
private:
    MacroCommandArgument arg;
};

/*============================================================================
================================ FormatMacroCommand ==========================
============================================================================*/

class FormatMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit FormatMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ DefMacroCommand =============================
============================================================================*/

class DefMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit DefMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ DefFMacroCommand ============================
============================================================================*/

class DefFMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit DefFMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ UndefMacroCommand ===========================
============================================================================*/

class UndefMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit UndefMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ DefinedMacroCommand =========================
============================================================================*/

class DefinedMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit DefinedMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ SetMacroCommand =============================
============================================================================*/

class SetMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit SetMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ SetFMacroCommand ============================
============================================================================*/

class SetFMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit SetFMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ GetMacroCommand =============================
============================================================================*/

class GetMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit GetMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ CallMacroCommand ============================
============================================================================*/

class CallMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit CallMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ VarMacroCommand =============================
============================================================================*/

class VarMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit VarMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ BinMacroCommand =============================
============================================================================*/

class BinMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit BinMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ UnMacroCommand ==============================
============================================================================*/

class UnMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit UnMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ CMacroCommand ===============================
============================================================================*/

class CMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit CMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ MultiMacroCommand ===========================
============================================================================*/

class MultiMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit MultiMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ForMacroCommand =============================
============================================================================*/

class ForMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ForMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ IfMacroCommand ==============================
============================================================================*/

class IfMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit IfMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ WaitMacroCommand ============================
============================================================================*/

class WaitMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit WaitMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ FindMacroCommand ============================
============================================================================*/

class FindMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit FindMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ReplaceMacroCommand =========================
============================================================================*/

class ReplaceMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ReplaceMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ReplaceSelMacroCommand ======================
============================================================================*/

class ReplaceSelMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ReplaceSelMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ReplaceDocMacroCommand ======================
============================================================================*/

class ReplaceDocMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ReplaceDocMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ExecMacroCommand ============================
============================================================================*/

class ExecMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ExecMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ExecFMacroCommand ===========================
============================================================================*/

class ExecFMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ExecFMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ExecDMacroCommand ===========================
============================================================================*/

class ExecDMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ExecDMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ ExecFDMacroCommand ===========================
============================================================================*/

class ExecFDMacroCommand : public MultiArgMacroCommand
{
public:
    static AbstractMacroCommand *create(const QList<MacroCommandArgument> &args);
private:
    explicit ExecFDMacroCommand(const QList<MacroCommandArgument> &args);
public:
    QString execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error = 0) const;
    QString name() const;
    QString toText() const;
    AbstractMacroCommand *clone() const;
};

/*============================================================================
================================ Global static functions =====================
============================================================================*/

static bool predLeqF(const double &t1, const double &t2)
{
    return t1 <= t2;
}

static bool predLeqI(const int &t1, const int &t2)
{
    return t1 <= t2;
}

static bool predGeqF(const double &t1, const double &t2)
{
    return t1 >= t2;
}

static bool predGeqI(const int &t1, const int &t2)
{
    return t1 >= t2;
}

static QString toRawText(QString s)
{
    s.replace("\\\\", "\\");
    s.replace("\\n", "\n");
    s.replace("\\t", "\t");
    s.replace("\\%", "%");
    s.replace("\\$", "$");
    s.remove("\\e");
    s.replace("\\{", "{");
    s.replace("\\}", "}");
    s.replace("\\[", "[");
    s.replace("\\]", "]");
    return s;
}

static QString toVisibleText(QString s)
{
    s.remove('\r');
    s.replace('\n', "\\n");
    s.replace('\t', "\\t");
    s.replace('\\', "\\\\");
    s.replace('%', "\\%");
    s.replace('$', "\\$");
    s.replace('{', "\\{");
    s.replace('}', "\\}");
    s.replace('[', "\\[");
    s.replace(']', "\\]");
    return s;
}

static QStringList getArgs(const QString &text, int &i, char opbr, char clbr, int max, char nopbr = '\0',
                           QString *error = 0)
{
    if (i >= text.length())
        return bRet(error, QString(), QStringList());
    if (text.isEmpty() || i < 0)
        return bRet(error, QString("Argument parcing error"), QStringList());
    QStringList args;
    int depth = 1;
    QString s;
    while (i < text.length() && (text.at(i) != nopbr ||  text.at(i - 1) == '\\' || depth))
    {
        if (text.at(i) == clbr && (text.at(i - 1) != '\\' || text.at(i - 2) == '\\'))
        {
            --depth;
            if (depth)
                s += clbr;
        }
        else if (text.at(i) == opbr && (text.at(i - 1) != '\\' || text.at(i - 2) == '\\'))
        {
            if (depth)
                s += opbr;
            ++depth;
        }
        else
        {   if (!depth)
                return bRet(error, QString("Argument parcing error"), QStringList());
            s += text.at(i);
        }
        if (!depth)
        {
            args << s;
            if (max >= 0 && args.size() > max)
                return bRet(error, QString("Too many arguments"), QStringList());
            s.clear();
        }
        ++i;
    }
    if (!s.isEmpty() || depth)
        return bRet(error, QString("Argument parcing error"), QStringList());
    return bRet(error, QString(), args);
}

static QString getReplaceMndParameters(const QList<MacroCommandArgument> &args, BAbstractCodeEditorDocument *doc,
                                       MacroExecutionStack *stack, QString &first, QString *second = 0)
{
    QString err;
    QString s1 = args.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return err;
    if (second)
    {
        QString s2 = args.at(1).toText(doc, stack, &err);
        if (!err.isEmpty())
            return err;
        *second = s2;
    }
    first = s1;
    return "";
}

static QString getReplaceOptParameters(const QList<MacroCommandArgument> &args, BAbstractCodeEditorDocument *doc,
                                       MacroExecutionStack *stack, int from, Qt::CaseSensitivity &cs,
                                       bool *w = 0, bool *bw = 0, bool *nc = 0)
{
    Qt::CaseSensitivity csx = Qt::CaseInsensitive;
    bool wx = false;
    bool bwx = false;
    bool ncx = false;
    bool csb = false;
    bool wb = false;
    bool bwb = false;
    bool ncb = false;
    for (int i = from; i < args.size(); ++i)
    {
        QString err;
        QString s = args.at(i).toText(doc, stack, &err);
        if (!err.isEmpty())
            return err;
        if (!s.compare("cs", Qt::CaseInsensitive) || !s.compare("case-sensitive", Qt::CaseInsensitive))
        {
            if (csb)
                return "Duplicate parameter";
            csx = Qt::CaseSensitive;
            csb = true;
        }
        else if (!s.compare("ci", Qt::CaseInsensitive) || !s.compare("case-insensitive", Qt::CaseInsensitive))
        {
            if (csb)
                return "Duplicate parameter";
            csx = Qt::CaseInsensitive;
            csb = true;
        }
        else if (!s.compare("w", Qt::CaseInsensitive) || !s.compare("words", Qt::CaseInsensitive)
                || !s.compare("whole-words", Qt::CaseInsensitive))
        {
            if (!w)
                return "Unknown parameter";
            if (wb)
                return "Duplicate parameter";
            wx = true;
            wb = true;
        }
        else if (!s.compare("bw", Qt::CaseInsensitive) || !s.compare("backward", Qt::CaseInsensitive)
                || !s.compare("backward-order", Qt::CaseInsensitive))
        {
            if (!bw)
                return "Unknown parameter";
            if (bwb)
                return "Duplicate parameter";
            bwx = true;
            bwb = true;
        }
        else if (!s.compare("nc", Qt::CaseInsensitive) || !s.compare("non-cyclic", Qt::CaseInsensitive))
        {
            if (!nc)
                return "Unknown parameter";
            if (ncb)
                return "Duplicate parameter";
            ncx = true;
            ncb = true;
        }
        else
        {
            return "Unknown parameter";
        }
    }
    cs = csx;
    bSet(w, wx);
    bSet(bw, bwx);
    bSet(nc, ncx);
    return "";
}

static QTextDocument::FindFlags createFindFlags(Qt::CaseSensitivity cs, bool w, bool bw)
{
    QTextDocument::FindFlags flags = 0;
    if (Qt::CaseSensitive == cs)
        flags |= QTextDocument::FindCaseSensitively;
    if (w)
        flags |= QTextDocument::FindWholeWords;
    if (bw)
        flags |= QTextDocument::FindBackward;
    return flags;
}

static QString getCommand(const MacroCommandArgument &arg, BAbstractCodeEditorDocument *doc,
                          MacroExecutionStack *stack, QString &command)
{
    QString err;
    QString cmd = arg.toText(doc, stack, &err);
    if (!err.isEmpty())
        return err;
    QString cmd2 = Global::externalTools().value(cmd);
    if (!cmd2.isEmpty())
        cmd = cmd2;
    if (cmd.isEmpty())
        return "Invalid parameter";
    command = cmd;
    return "";
}

static QString getWorkingDir(BAbstractCodeEditorDocument *doc, QString &dir)
{
    QFileInfo fi(doc->fileName());
    if (!fi.exists() || !fi.isFile())
        return "Document does not exist on disk";
    dir = fi.path();
    return "";
}

static QString getFileName(const MacroCommandArgument &arg, BAbstractCodeEditorDocument *doc,
                           MacroExecutionStack *stack, QString &fileName, QString *workingDir = 0)
{
    QString path;
    QString err = getWorkingDir(doc, path);
    if (!err.isEmpty())
        return err;
    QString fn = path + "/" + arg.toText(doc, stack, &err);
    if (!err.isEmpty())
        return err;
    if (fn.isEmpty())
        return "Invalid parameter";
    QFileInfo fi(fn);
    if (!fi.exists() || !fi.isFile())
        return "File does not exist";
    fileName = fn;
    bSet(workingDir, path);
    return "";
}

static QString getCommandArgs(const QList<MacroCommandArgument> &args, int from, BAbstractCodeEditorDocument *doc,
                              MacroExecutionStack *stack, QStringList &arguments)
{
    QStringList sl;
    for (int i = from; i < args.size(); ++i)
    {
        QString err;
        QString s = args.at(i).toText(doc, stack, &err);
        if (!err.isEmpty())
            return err;
        sl << s;
    }
    arguments = sl;
    return "";
}

static QString processOutputToText(const QByteArray &output)
{
    QTextCodec *codec = BTextTools::guessTextCodec(output, Application::locale());
    if (!codec)
        return toVisibleText(QString(output));
    QTextStream in(output);
    in.setCodec(codec);
    return toVisibleText(in.readAll());
}

static QString toDouble(const QString &text, double &d, bool *native = 0)
{
    if (text.isEmpty())
        return bRet(native, false, QString("Failed to convert"));
    if (text.toLower() == "true")
    {
        d = 1.0;
        return bRet(native, false, QString());
    }
    else if (text.toLower() == "false")
    {
        d = 0.0;
        return bRet(native, false, QString());
    }
    if (text.contains('.'))
    {
        bool ok = false;
        double dd = text.toDouble(&ok);
        if (!ok)
            return bRet(native, true, QString("Failed to convert"));
        d = dd;
        return bRet(native, true, QString());
    }
    else
    {
        bool ok = false;
        int i = text.toInt(&ok);
        if (!ok)
            return bRet(native, false, QString("Failed to convert"));
        d = (double) i;
        return bRet(native, false, QString());
    }
}

static QString toInt(const QString &text, int &i, bool *native = 0)
{
    if (text.isEmpty())
        return bRet(native, false, QString("Failed to convert"));
    if (text.toLower() == "true")
    {
        i = 1;
        return bRet(native, false, QString());
    }
    else if (text.toLower() == "false")
    {
        i = 0;
        return bRet(native, false, QString());
    }
    if (text.contains('.'))
    {
        bool ok = false;
        double d = text.toDouble(&ok);
        if (!ok)
            return bRet(native, false, QString("Failed to convert"));
        i = (int) d;
        return bRet(native, false, QString());
    }
    else
    {
        bool ok = false;
        int ii = text.toInt(&ok);
        if (!ok)
            return bRet(native, true, QString("Failed to convert"));
        i = ii;
        return bRet(native, true, QString());
    }
}

static QString toBool(const QString &text, bool &b, bool *native = 0)
{
    if (text.isEmpty())
        return bRet(native, false, QString("Failed to convert"));
    if (text.toLower() == "true")
    {
        b = true;
        return bRet(native, true, QString());
    }
    else if (text.toLower() == "false")
    {
        b = false;
        return bRet(native, true, QString());
    }
    if (text.contains('.'))
    {
        bool ok = false;
        double d = text.toDouble(&ok);
        if (!ok)
            return bRet(native, false, QString("Failed to convert"));
        b = (bool) d;
        return bRet(native, false, QString());
    }
    else
    {
        bool ok = false;
        int i = text.toInt(&ok);
        if (!ok)
            return bRet(native, false, QString("Failed to convert"));
        b = (bool) i;
        return bRet(native, false, QString());
    }
}

static QString formatText(QString &text, const QString &format = QString())
{
    if (text.isEmpty() || format.isEmpty())
        return "";
    if (QRegExp("i(\\.([1-9]|1[0-5]))?").exactMatch(format))
    {
        QStringList sl = format.split('.');
        int prec = 0;
        if (sl.size() == 2)
            prec = sl.last().toInt();
        bool ok = false;
        int i = (text.contains('.') || "inf" == text) ? (int) text.toDouble(&ok) : text.toInt(&ok);
        if (!ok)
            return "Unable to convert";
        text = QString::number(i);
        if (prec)
            text += "." + QString().fill('0', prec);
    }
    else if ("b" == format.toLower())
    {
        bool b = false;
        QString err = toBool(text, b);
        if (!err.isEmpty())
            return err;
        text = b ? "true" : "false";
        return "";
    }
    else if (QRegExp("(f|e|E|g|G)(\\.([1-9]|1[0-5]))?").exactMatch(format))
    {
        QStringList sl = format.split('.');
        char f = sl.first().at(0).toAscii();
        int prec = 6;
        if (sl.size() == 2)
            prec = sl.last().toInt();
        bool ok = false;
        double d = text.toDouble(&ok);
        if (!ok)
            return "Unable to convert";
        text = QString::number(d, f, prec);
    }
    else
    {
        return "Unknown format";
    }
    return "";
}

static QString getVarDocName(BAbstractCodeEditorDocument *doc, MacroExecutionStack *, QString &v)
{
    v = QFileInfo(doc->fileName()).fileName();
    return "";
}

static QString getVarDocPath(BAbstractCodeEditorDocument *doc, MacroExecutionStack *, QString &v)
{
    QFileInfo fi(doc->fileName());
    if (!fi.isAbsolute())
        return "";
    v = fi.filePath();
    return "";
}

static QString getVarDocDir(BAbstractCodeEditorDocument *doc, MacroExecutionStack *, QString &v)
{
    QFileInfo fi(doc->fileName());
    if (!fi.isAbsolute())
        return "";
    v = fi.path();
    return "";
}

static QString unaryMinus(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool b = false;
    QString err = toInt(text, i, &b);
    if (!err.isEmpty())
        return err;
    if (b)
    {
        text = QString::number(-1 * i);
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(-1.0 * d, 'g', 15);
    return "";
}

static QString unaryNegation(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    bool b = false;
    QString err = toBool(text, b);
    if (!err.isEmpty())
        return err;
    text = b ? "false" : "true";
    return "";
}

static QString unaryLn(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::log(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::log(d), 'g', 15);
    return "";
}

static QString unaryLg(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::log10(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::log10(d), 'g', 15);
    return "";
}

static QString unarySqrt(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (i < 0)
            return "Taking square root of negative number";
        text = QString::number((int) std::sqrt(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    if (d < 0.0)
        return "Taking square root of negative number";
    text = QString::number(std::sqrt(d), 'g', 15);
    return "";
}

static QString unarySin(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::sin(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::sin(d), 'g', 15);
    return "";
}

static QString unaryCos(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::cos(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::cos(d), 'g', 15);
    return "";
}

static QString unaryTan(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        //TODO: Check
        text = QString::number((int) std::tan(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::tan(d), 'g', 15);
    return "";
}

static QString unaryCot(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        //TODO: Check
        text = QString::number(ctan(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(ctan(d), 'g', 15);
    return "";
}

static QString unarySec(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        //TODO: Check
        text = QString::number(sec(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(csc(d), 'g', 15);
    return "";
}

static QString unaryCsc(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        //TODO: Check
        text = QString::number((int) (1 / std::cos(i)));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(1 / std::cos(d), 'g', 15);
    return "";
}

static QString unaryAsin(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::asin(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::asin(d), 'g', 15);
    return "";
}

static QString unaryAcos(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::acos(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::acos(d), 'g', 15);
    return "";
}

static QString unaryAtan(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::atan(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::atan(d), 'g', 15);
    return "";
}

static QString unaryAcot(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(acot(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(acot(d), 'g', 15);
    return "";
}

static QString unaryAsec(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (!i)
            return "Taking asec of zero";
        text = QString::number(asec(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(asec(d), 'g', 15);
    return "";
}

static QString unaryAcsc(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (!i)
            return "Taking acsc of zero";
        text = QString::number(acsc(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(acsc(d), 'g', 15);
    return "";
}

static QString unarySh(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::sinh(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::sinh(d), 'g', 15);
    return "";
}

static QString unaryCh(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::cosh(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::cosh(d), 'g', 15);
    return "";
}

static QString unaryTh(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number((int) std::tanh(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(std::tanh(d), 'g', 15);
    return "";
}

static QString unaryCth(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(cth(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(cth(d), 'g', 15);
    return "";
}

static QString unarySech(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(sech(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(sech(d), 'g', 15);
    return "";
}

static QString unaryCsch(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(csch(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(csch(d), 'g', 15);
    return "";
}

static QString unaryArsh(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(arsh(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arsh(d), 'g', 15);
    return "";
}

static QString unaryArch(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (i < 1)
            return "Negative ln argument";
        text = QString::number(arch(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arch(d), 'g', 15);
    return "";
}

static QString unaryArth(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (1 == i)
            return "Division by zero";
        text = QString::number(arth(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arth(d), 'g', 15);
    return "";
}

static QString unaryArcth(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        if (1 == i)
            return "Division by zero";
        text = QString::number(arcth(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arcth(d), 'g', 15);
    return "";
}

static QString unaryArsch(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(arsch(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arsch(d), 'g', 15);
    return "";
}

static QString unaryArcsch(QString &text)
{
    if (text.isEmpty())
        return "Invalid value";
    int i = 0;
    bool native = false;
    QString err = toInt(text, i, &native);
    if (!err.isEmpty())
        return err;
    if (native)
    {
        text = QString::number(arcsch(i));
        return "";
    }
    double d = 0.0;
    err = toDouble(text, d);
    if (!err.isEmpty())
        return err;
    text = QString::number(arcsch(d), 'g', 15);
    return "";
}

static QString binaryAddition(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = text1 + text2;
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = text1 + text2;
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = text1 + text2;
        return "";
    }
    if (native1 && native2)
    {
        text1 = QString::number(i1 + i2);
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = QString::number(d1 + d2, 'g', 15);
    return "";
}

static QString binarySubtraction(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        text1 = QString::number(i1 - i2);
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = QString::number(d1 - d2, 'g', 15);
    return "";
}

static QString binaryMultiplication(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        text1 = QString::number(i1 * i2);
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = QString::number(d1 * d2, 'g', 15);
    return "";
}

static QString binaryDivision(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        if (!i2)
            return "Division by zero";
        text1 = QString::number(i1 / i2);
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = QString::number(d1 / d2, 'g', 15);
    return "";
}

static QString binaryInvolution(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        if (!i1)
            return "Involution of zero";
        text1 = QString::number(std::pow(i1, i2));
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = QString::number(std::pow(d1, d2), 'g', 15);
    return "";
}

static QString binaryLog(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        if (i1 <= 0 || i2 <= 0)
            return "Invalid log base/power";
        text1 = QString::number(anyLog(i1, i2));
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    if (d1 <= 0 || d2 <= 0)
        return "Invalid log base/power";
    text1 = QString::number(anyLog(d1, d2), 'g', 15);
    return "";
}

static QString binaryRoot(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
        return err;
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
        return err;
    if (native1 && native2)
    {
        if (i1 < 0 || i2 < 0)
            return "Invalid root base/power";
        text1 = QString::number(root(i1, i2));
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    if (d1 < 0 || d2 < 0)
        return "Invalid root base/power";
    text1 = QString::number(root(d1, d2), 'g', 15);
    return "";
}

static QString binaryEqual(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 == text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 == text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 == text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 == i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = (d1 == d2) ? "true" : "false";
    return "";
}

static QString binaryNotEqual(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 != text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 != text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 != text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 != i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = (d1 != d2) ? "true" : "false";
    return "";
}

static QString binaryLesser(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 < text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 < text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 < text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 < i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = (d1 < d2) ? "true" : "false";
    return "";
}

static QString binaryLesserOrEqual(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 <= text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 <= text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 <= text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 <= i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = (d1 <= d2) ? "true" : "false";
    return "";
}

static QString binaryGreater(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 > text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 > text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 > text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 > i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = (d1 > d2) ? "true" : "false";
    return "";
}

static QString binaryGreaterOrEqual(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
    {
        text1 = (text1 >= text2) ? "true" : "false";
        return "";
    }
    int i1 = 0;
    bool native1 = false;
    QString err = toInt(text1, i1, &native1);
    if (!err.isEmpty())
    {
        text1 = (text1 >= text2) ? "true" : "false";
        return "";
    }
    int i2 = 0;
    bool native2 = false;
    err = toInt(text2, i2, &native2);
    if (!err.isEmpty())
    {
        text1 = (text1 >= text2) ? "true" : "false";
        return "";
    }
    if (native1 && native2)
    {
        text1 = (i1 >= i2) ? "true" : "false";
        return "";
    }
    double d1 = 0.0;
    err = toDouble(text1, d1);
    double d2 = 0.0;
    err = toDouble(text2, d2);
    text1 = (d1 >= d2) ? "true" : "false";
    return "";
}

static QString binaryAnd(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    bool b1 = false;
    QString err = toBool(text1, b1);
    if (!err.isEmpty())
        return err;
    bool b2 = false;
    err = toBool(text2, b2);
    if (!err.isEmpty())
        return err;
    text1 = (b1 && b2) ? "true" : "false";
    return "";
}

static QString binaryOr(QString &text1, const QString &text2)
{
    if (text1.isEmpty() || text2.isEmpty())
        return "Invalid value";
    bool b1 = false;
    QString err = toBool(text1, b1);
    if (!err.isEmpty())
        return err;
    bool b2 = false;
    err = toBool(text2, b2);
    if (!err.isEmpty())
        return err;
    text1 = (b1 || b2) ? "true" : "false";
    return "";
}

/*============================================================================
================================ AbstractMacroCommand ========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *AbstractMacroCommand::fromText(QString text, QString *error)
{
    typedef QMap<QString, FunctionInfo> FunctionInfoMap;
    init_once(FunctionInfoMap, infoMap, FunctionInfoMap())
    {
        infoMap.insert("insert", FunctionInfo(&InsertMacroCommand::create, 1));
        infoMap.insert("press", FunctionInfo(&PressMacroCommand::create, 1));
        infoMap.insert("format", FunctionInfo(&FormatMacroCommand::create, 2));
        infoMap.insert("def", FunctionInfo(&DefMacroCommand::create, 2, 1));
        infoMap.insert("defF", FunctionInfo(&DefFMacroCommand::create, 2, 1));
        infoMap.insert("undef", FunctionInfo(&UndefMacroCommand::create, 1));
        infoMap.insert("defined", FunctionInfo(&DefinedMacroCommand::create, 1));
        infoMap.insert("set", FunctionInfo(&SetMacroCommand::create, 2, 1));
        infoMap.insert("setF", FunctionInfo(&SetFMacroCommand::create, 2));
        infoMap.insert("get", FunctionInfo(&GetMacroCommand::create, 1, 1));
        infoMap.insert("call", FunctionInfo(&CallMacroCommand::create, 1, -1));
        infoMap.insert("var", FunctionInfo(&VarMacroCommand::create, 1, 1));
        infoMap.insert("bin", FunctionInfo(&BinMacroCommand::create, 3, 1));
        infoMap.insert("un", FunctionInfo(&UnMacroCommand::create, 2, 1));
        infoMap.insert("c", FunctionInfo(&CMacroCommand::create, 1, 1));
        infoMap.insert("multi", FunctionInfo(&MultiMacroCommand::create, 1, -1));
        infoMap.insert("for", FunctionInfo(&ForMacroCommand::create, 5, -1));
        infoMap.insert("if", FunctionInfo(&IfMacroCommand::create, 2, 1));
        infoMap.insert("wait", FunctionInfo(&WaitMacroCommand::create, 1, 2));
        infoMap.insert("find", FunctionInfo(&FindMacroCommand::create, 1, 4));
        infoMap.insert("replace", FunctionInfo(&ReplaceMacroCommand::create, 2, 4));
        infoMap.insert("replaceSel", FunctionInfo(&ReplaceSelMacroCommand::create, 2, 1));
        infoMap.insert("replaceDoc", FunctionInfo(&ReplaceDocMacroCommand::create, 2, 1));
        infoMap.insert("exec", FunctionInfo(&ExecMacroCommand::create, 1, -1));
        infoMap.insert("execF", FunctionInfo(&ExecFMacroCommand::create, 2, -1));
        infoMap.insert("execD", FunctionInfo(&ExecDMacroCommand::create, 1, -1));
        infoMap.insert("execFD", FunctionInfo(&ExecFDMacroCommand::create, 2, -1));
    }
    int ind = Global::indexOfHelper(text, "%");
    if (ind >= 0)
        text.remove(ind, text.length() - ind);
    if (text.isEmpty())
        return bRet(error, QString("Empty string"), (AbstractMacroCommand *) 0);
    if (!text.startsWith('\\'))
        return bRet(error, QString("A command must start with \"\\\""), (AbstractMacroCommand *) 0);
    QString commandName;
    int i = Global::indexOfHelper(text, "{", 2);
    commandName += text.mid(1, (i > 0) ? (i - 1) : -1);
    FunctionInfo info = infoMap.value(commandName);
    if (!info.isValid())
        return bRet(error, QString("Invalid function name"), (AbstractMacroCommand *) 0);
    QString err;
    QStringList textArgs = getArgs(text, ++i, '{', '}', info.mndArgCount, '[', &err);
    if (!err.isEmpty())
        return bRet(error, err, (AbstractMacroCommand *) 0);
    if (i + 1 == text.length() && (text.at(i) == '[' || text.at(i) == '{'))
        return bRet(error, QString("Argument parcing error"), (AbstractMacroCommand *) 0);
    textArgs += getArgs(text, ++i, '[', ']', info.optArgCount, '\0', &err);
    if (!err.isEmpty())
        return bRet(error, err, (AbstractMacroCommand *) 0);
    if (i < text.length())
        return bRet(error, QString("Argument parcing error"), (AbstractMacroCommand *) 0);
    QList<MacroCommandArgument> args;
    foreach (const QString &t, textArgs)
    {
        MacroCommandArgument arg(t);
        if (!arg.isValid())
            return bRet(error, arg.error(), (AbstractMacroCommand *) 0);
        args << arg;
    }
    return bRet(error, QString(), info.func(args));
}

AbstractMacroCommand *AbstractMacroCommand::fromKeyPressEvent(const QKeyEvent *e,
                                                              AbstractMacroCommand *previousCommand)
{
    return fromKeyPressEvent(e, 0, previousCommand);
}

AbstractMacroCommand *AbstractMacroCommand::fromKeyPressEvent(const QKeyEvent *e, QString *error,
                                                              AbstractMacroCommand *previousCommand)
{
    if (!e || e->type() != QEvent::KeyPress)
        return bRet(error, QString("The event is not a key press"), (AbstractMacroCommand *) 0);
    int key = e->key();
    Qt::KeyboardModifiers modifiers = e->modifiers();
    QString text = e->text();
    if (Qt::Key_Control == key || Qt::Key_Alt == key || Qt::Key_Shift == key)
        return bRet(error, QString("The event is a modifier key press"), (AbstractMacroCommand *) 0);
    if (text.isEmpty() || !text.at(0).isPrint() || text.at(0).isSpace()
            || (modifiers & Qt::ControlModifier) || (modifiers & Qt::AltModifier))
        return bRet(error, QString(), new PressMacroCommand(key, modifiers));
    if (!previousCommand || previousCommand->name() != "insert")
        return bRet(error, QString(), new InsertMacroCommand(text));
    if (!dynamic_cast<InsertMacroCommand *>(previousCommand)->append(text))
        return bRet(error, QString(), new InsertMacroCommand(text));
    return bRet(error, QString(), previousCommand);
}

/*============================== Public constructors =======================*/

AbstractMacroCommand::AbstractMacroCommand()
{
    //
}

AbstractMacroCommand::~AbstractMacroCommand()
{
    //
}

/*============================== Public methods ============================*/

void AbstractMacroCommand::clear()
{
    //
}

bool AbstractMacroCommand::compare(const AbstractMacroCommand *other) const
{
    if (!other || other->name() != name())
        return false;
    return other && other->name() == name() && compareInternal(other);
}

/*============================================================================
================================ FunctionInfo ================================
============================================================================*/

/*============================== Public constructors =======================*/

FunctionInfo::FunctionInfo()
{
    func = 0;
    mndArgCount = 0;
    optArgCount = 0;
}
FunctionInfo::FunctionInfo(Function f, int mnd, int opt)
{
    func = f;
    mndArgCount = mnd;
    optArgCount = opt;
}

/*============================== Public methods ============================*/

bool FunctionInfo::isValid() const
{
    return func && mndArgCount >= 0;
}

/*============================================================================
================================ ThreadHack ==================================
============================================================================*/

/*============================== Static public methods =====================*/

void ThreadHack::waitBlocking(int msecs)
{
    currentThread()->msleep(msecs);
}

/*============================================================================
================================ MultiArgMacroCommand ========================
============================================================================*/

/*============================== Public constructors =======================*/

MultiArgMacroCommand::~MultiArgMacroCommand()
{
    //
}

/*============================== Protected constructors ====================*/

MultiArgMacroCommand::MultiArgMacroCommand(const QList<MacroCommandArgument> &args)
{
    margs = args;
}

/*============================== Public methods ============================*/

void MultiArgMacroCommand::clear()
{
    margs.clear();
}

bool MultiArgMacroCommand::isValid() const
{
    if (margs.isEmpty())
        return false;
    foreach (const MacroCommandArgument &a, margs)
        if (!a.isValid())
            return false;
    return true;
}

/*============================== Protected methods =========================*/

bool MultiArgMacroCommand::compareInternal(const AbstractMacroCommand *other) const
{
    const MultiArgMacroCommand *o = dynamic_cast<const MultiArgMacroCommand *>(other);
    return o && margs == o->margs;
}

/*============================================================================
================================ InsertMacroCommand ==========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *InsertMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 1) ? new InsertMacroCommand(args.first()) : 0;
}

/*============================== Public constructors =======================*/

InsertMacroCommand::InsertMacroCommand(const QString &txt)
{
    arg.fromText(txt);
}

InsertMacroCommand::~InsertMacroCommand()
{
    //
}

/*============================== Private constructors ======================*/

InsertMacroCommand::InsertMacroCommand(const MacroCommandArgument &a)
{
    arg = a;
}

/*============================== Public methods ============================*/

void InsertMacroCommand::clear()
{
    arg.clear();
}

QString InsertMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !arg.isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString text = arg.toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    doc->insertText(toRawText(text));
    return bRet(error, QString(), QString("true"));
}

QString InsertMacroCommand::name() const
{
    return "insert";
}

QString InsertMacroCommand::toText() const
{
    return arg.isValid() ? ("\\insert{" + arg.toText() + "}") : QString();
}

bool InsertMacroCommand::isValid() const
{
    return arg.isValid();
}

AbstractMacroCommand *InsertMacroCommand::clone() const
{
    return new InsertMacroCommand(arg);
}

bool InsertMacroCommand::append(const QString &txt)
{
    if (txt.isEmpty() || !arg.isText())
        return false;
    return arg.fromText(arg.toText() + txt);
}

/*============================== Protected methods =========================*/

bool InsertMacroCommand::compareInternal(const AbstractMacroCommand *other) const
{
    const InsertMacroCommand *o = dynamic_cast<const InsertMacroCommand *>(other);
    return o && arg == o->arg;
}

/*============================================================================
================================ PressMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *PressMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 1) ? new PressMacroCommand(args.first()) : 0;
}

/*============================== Public constructors =======================*/

PressMacroCommand::PressMacroCommand(int k, Qt::KeyboardModifiers m)
{
    arg.fromText(QKeySequence(k | m).toString(QKeySequence::PortableText));
}

PressMacroCommand::~PressMacroCommand()
{
    //
}

/*============================== Private constructors ======================*/

PressMacroCommand::PressMacroCommand(const MacroCommandArgument &a)
{
    arg = a;
}

/*============================== Public methods ============================*/

void PressMacroCommand::clear()
{
    arg.clear();
}

QString PressMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !arg.isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QPlainTextEdit *ptedt = doc->findChild<QPlainTextEdit *>();
    if (!ptedt)
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QKeySequence ks(arg.toText(doc, stack, &err));
    if (ks.isEmpty() || !err.isEmpty())
        return bRet(error, !err.isEmpty() ? err : QString("Invalid parameter"), QString("false"));
    for (int i = 0; i < (int) ks.count(); ++i)
    {
        int key = ~Qt::KeyboardModifierMask & ks[i];
        Qt::KeyboardModifiers modifiers = static_cast<Qt::KeyboardModifiers>(Qt::KeyboardModifierMask & ks[i]);
        QCoreApplication::postEvent(ptedt, new QKeyEvent(QEvent::KeyPress, key, modifiers));
    }
    return bRet(error, QString(), QString("true"));
}

QString PressMacroCommand::name() const
{
    return "press";
}

QString PressMacroCommand::toText() const
{
    return arg.isValid() ? ("\\press{" + arg.toText() + "}") : QString();
}

bool PressMacroCommand::isValid() const
{
    return arg.isValid();
}

AbstractMacroCommand *PressMacroCommand::clone() const
{
    return new PressMacroCommand(arg);
}

/*============================== Protected methods =========================*/

bool PressMacroCommand::compareInternal(const AbstractMacroCommand *other) const
{
    const PressMacroCommand *o = dynamic_cast<const PressMacroCommand *>(other);
    return o && arg == o->arg;
}

/*============================================================================
================================ FormatMacroCommand ==========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *FormatMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 2) ? new FormatMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

FormatMacroCommand::FormatMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString FormatMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString f = margs.last().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    err = formatText(s, f);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), s);
}

QString FormatMacroCommand::name() const
{
    return "format";
}

QString FormatMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\fromat{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    return s;
}

AbstractMacroCommand *FormatMacroCommand::clone() const
{
    return new FormatMacroCommand(margs);
}

/*============================================================================
================================ DefMacroCommand =============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *DefMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new DefMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

DefMacroCommand::DefMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString DefMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString v = margs.at(1).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    bool g = false;
    if (margs.size() == 3)
    {
        QString ss = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
        if (!ss.compare("g", Qt::CaseInsensitive) || !ss.compare("global", Qt::CaseInsensitive))
            g = true;
        else if (ss.compare("l", Qt::CaseInsensitive) && ss.compare("local", Qt::CaseInsensitive))
            return bRet(error, QString("Unknown parameter"), QString());
    }
    if (!stack->define(s, v, g))
        return bRet(error, QString("Variable redeclaration"), QString());
    return bRet(error, QString(), s);
}

QString DefMacroCommand::name() const
{
    return "def";
}

QString DefMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\def{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *DefMacroCommand::clone() const
{
    return new DefMacroCommand(margs);
}

/*============================================================================
================================ DefFMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *DefFMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new DefFMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

DefFMacroCommand::DefFMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString DefFMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString v = margs.at(1).toText();
    bool g = false;
    if (margs.size() == 3)
    {
        QString ss = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
        if (!ss.compare("g", Qt::CaseInsensitive) || !ss.compare("global", Qt::CaseInsensitive))
            g = true;
        else if (ss.compare("l", Qt::CaseInsensitive) && ss.compare("local", Qt::CaseInsensitive))
            return bRet(error, QString("Unknown parameter"), QString());
    }
    if (!stack->defineF(s, v, g))
        return bRet(error, QString("Variable redeclaration"), QString());
    return bRet(error, QString(), s);
}

QString DefFMacroCommand::name() const
{
    return "defF";
}

QString DefFMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\defF{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *DefFMacroCommand::clone() const
{
    return new DefFMacroCommand(margs);
}

/*============================================================================
================================ UndefMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *UndefMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 1) ? new UndefMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

UndefMacroCommand::UndefMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString UndefMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    if (!stack->undefine(s))
        return bRet(error, QString("No such variable"), QString("false"));
    return bRet(error, QString(), QString("true"));
}

QString UndefMacroCommand::name() const
{
    return "undef";
}

QString UndefMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\undef{" + margs.first().toText() + "}";
    return s;
}

AbstractMacroCommand *UndefMacroCommand::clone() const
{
    return new UndefMacroCommand(margs);
}

/*============================================================================
================================ DefinedMacroCommand =========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *DefinedMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 1) ? new DefinedMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

DefinedMacroCommand::DefinedMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString DefinedMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack,
                                     QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    return bRet(error, QString(), QString(stack->isDefined(s) ? "true" : "false"));
}

QString DefinedMacroCommand::name() const
{
    return "defined";
}

QString DefinedMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\defined{" + margs.first().toText() + "}";
    return s;
}

AbstractMacroCommand *DefinedMacroCommand::clone() const
{
    return new DefinedMacroCommand(margs);
}

/*============================================================================
================================ SetMacroCommand =============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *SetMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2  && args.size() <= 3) ? new SetMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

SetMacroCommand::SetMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString SetMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString v = margs.at(1).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString f;
    if (margs.size() == 3)
    {
        f = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    if (!stack->set(s, v))
        return bRet(error, QString("No such variable"), QString());
    err = formatText(v, f);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), v);
}

QString SetMacroCommand::name() const
{
    return "set";
}

QString SetMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\set{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *SetMacroCommand::clone() const
{
    return new SetMacroCommand(margs);
}

/*============================================================================
================================ SetFMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *SetFMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() == 2) ? new SetFMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

SetFMacroCommand::SetFMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString SetFMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString v = margs.at(1).toText();
    QString f;
    if (margs.size() == 3)
    {
        f = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    if (!stack->setF(s, v))
        return bRet(error, QString("No such variable"), QString());
    return bRet(error, QString(), v);
}

QString SetFMacroCommand::name() const
{
    return "setF";
}

QString SetFMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\setF{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    return s;
}

AbstractMacroCommand *SetFMacroCommand::clone() const
{
    return new SetFMacroCommand(margs);
}

/*============================================================================
================================ GetMacroCommand =============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *GetMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1 && args.size() <= 2) ? new GetMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

GetMacroCommand::GetMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString GetMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString f;
    if (margs.size() == 2)
    {
        f = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    QString v;
    if (!stack->get(s, v))
        return bRet(error, QString("No such variable"), QString());
    err = formatText(v, f);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), v);
}

QString GetMacroCommand::name() const
{
    return "get";
}

QString GetMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\get{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *GetMacroCommand::clone() const
{
    return new GetMacroCommand(margs);
}

/*============================================================================
================================ CallMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *CallMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1) ? new CallMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

CallMacroCommand::CallMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString CallMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString v;
    if (!stack->getF(s, v))
        return bRet(error, QString("No such variable"), QString());
    for (int i = 1; i < margs.size(); ++i)
    {
        QString si = "$" + QString::number(i);
        int ind = Global::indexOfHelper(v, si);
        if (ind < 0)
            return bRet(error, QString("Argument count mismatch"), QString());
        QString vv = margs.at(i).toText();
        while (ind >= 0)
        {
            v.replace(ind, si.length(), vv);
            ind = Global::indexOfHelper(v, si);
        }
    }
    AbstractMacroCommand *c = AbstractMacroCommand::fromText(v, &err);
    if (!c || !err.isEmpty())
        return bRet(error, err, QString());
    QString ss = c->execute(doc, stack, &err);
    delete c;
    return bRet(error, err, ss);
}

QString CallMacroCommand::name() const
{
    return "call";
}

QString CallMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\call{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *CallMacroCommand::clone() const
{
    return new CallMacroCommand(margs);
}

/*============================================================================
================================ VarMacroCommand =============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *VarMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1 && args.size() <= 2) ? new VarMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

VarMacroCommand::VarMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString VarMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    typedef QMap<QString, QString(*)(BAbstractCodeEditorDocument *, MacroExecutionStack *, QString &)> FunctionMap;
    init_once(FunctionMap, funcMap, FunctionMap())
    {
        funcMap.insert("docName", &getVarDocName);
        funcMap.insert("docPath", &getVarDocPath);
        funcMap.insert("docDir", &getVarDocDir);
    }
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    if (!funcMap.contains(s))
        return bRet(error, QString("Unknown operator"), QString());
    QString f;
    if (margs.size() == 2)
    {
        f = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    QString v;
    err = funcMap[s](doc, stack, v);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    err = formatText(v, f);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), v);
}

QString VarMacroCommand::name() const
{
    return "var";
}

QString VarMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\var{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *VarMacroCommand::clone() const
{
    return new VarMacroCommand(margs);
}

/*============================================================================
================================ BinMacroCommand =============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *BinMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 3 && args.size() <= 4) ? new BinMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

BinMacroCommand::BinMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString BinMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    typedef QMap<QString, QString (*)(QString &, const QString &)> BinaryFuncMap;
    init_once(BinaryFuncMap, funcMap, BinaryFuncMap())
    {
        funcMap.insert("+", &binaryAddition);
        funcMap.insert("-", &binarySubtraction);
        funcMap.insert("*", &binaryMultiplication);
        funcMap.insert("/", &binaryDivision);
        funcMap.insert("^", &binaryInvolution);
        funcMap.insert("log", &binaryLog);
        funcMap.insert("root", &binaryRoot);
        funcMap.insert("==", &binaryEqual);
        funcMap.insert("!=", &binaryNotEqual);
        funcMap.insert("<", &binaryLesser);
        funcMap.insert("<=", &binaryLesserOrEqual);
        funcMap.insert(">", &binaryGreater);
        funcMap.insert(">=", &binaryGreaterOrEqual);
        funcMap.insert("||", &binaryOr);
        funcMap.insert("&&", &binaryAnd);
    }
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    if (!funcMap.contains(s))
        return bRet(error, QString("Unknown operator"), QString());
    QString v1 = margs.at(1).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString v2 = margs.at(2).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString f;
    if (margs.size() == 4)
    {
        f = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    err = funcMap[s](v1, v2);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    err = formatText(v1, f);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), v1);
}

QString BinMacroCommand::name() const
{
    return "bin";
}

QString BinMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\bin{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}{" + margs.at(2).toText() + "}";
    for (int i = 3; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *BinMacroCommand::clone() const
{
    return new BinMacroCommand(margs);
}

/*============================================================================
================================ UnMacroCommand ==============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *UnMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new UnMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

UnMacroCommand::UnMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString UnMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    typedef QMap<QString, QString (*)(QString &)> UnaryFuncMap;
    init_once(UnaryFuncMap, funcMap, UnaryFuncMap())
    {
        funcMap.insert("-", &unaryMinus);
        funcMap.insert("!", &unaryNegation);
        funcMap.insert("ln", &unaryLn);
        funcMap.insert("lg", &unaryLg);
        funcMap.insert("sqrt", &unarySqrt);
        funcMap.insert("sin", &unarySin);
        funcMap.insert("cos", &unaryCos);
        funcMap.insert("tan", &unaryTan);
        funcMap.insert("cot", &unaryCot);
        funcMap.insert("sec", &unarySec);
        funcMap.insert("csc", &unaryCsc);
        funcMap.insert("asin", &unaryAsin);
        funcMap.insert("acos", &unaryAcos);
        funcMap.insert("atan", &unaryAtan);
        funcMap.insert("acot", &unaryAcot);
        funcMap.insert("asec", &unaryAsec);
        funcMap.insert("acsc", &unaryAcsc);
        funcMap.insert("sh", &unarySh);
        funcMap.insert("ch", &unaryCh);
        funcMap.insert("th", &unaryTh);
        funcMap.insert("cth", &unaryCth);
        funcMap.insert("sech", &unarySech);
        funcMap.insert("csch", &unaryCsch);
        funcMap.insert("arsh", &unaryArsh);
        funcMap.insert("arch", &unaryArch);
        funcMap.insert("arth", &unaryArth);
        funcMap.insert("arcth", &unaryArcth);
        funcMap.insert("arsch", &unaryArsch);
        funcMap.insert("arcsch", &unaryArcsch);
    }
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    if (!funcMap.contains(s))
        return bRet(error, QString("Unknown operator"), QString());
    QString v = margs.at(1).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString f;
    if (margs.size() == 3)
    {
        f = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    err = funcMap[s](v);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    err = formatText(v, f);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), v);
}

QString UnMacroCommand::name() const
{
    return "un";
}

QString UnMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\un{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *UnMacroCommand::clone() const
{
    return new UnMacroCommand(margs);
}

/*============================================================================
================================ CMacroCommand ===============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *CMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1 && args.size() <= 2) ? new CMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

CMacroCommand::CMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString CMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    typedef QMap<QString, QString> StringMap;
    init_once(StringMap, constMap, StringMap())
    {
        constMap.insert("pi", "3.14159265358979");
        constMap.insert("e", "2.71828182845904");
        constMap.insert("G", "6.67348808080808");
        constMap.insert("c", "299792458");
    }
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    if (!constMap.contains(s))
        return bRet(error, QString("Unknown constant"), QString());
    QString f;
    if (margs.size() == 2)
    {
        f = margs.last().toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    QString v = constMap.value(s);
    err = formatText(v, f);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), v);
}

QString CMacroCommand::name() const
{
    return "c";
}

QString CMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\c{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *CMacroCommand::clone() const
{
    return new CMacroCommand(margs);
}

/*============================================================================
================================ MultiMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *MultiMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1) ? new MultiMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

MultiMacroCommand::MultiMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString MultiMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    foreach (const MacroCommandArgument &a, margs)
    {
        QString err;
        a.toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString());
    }
    return bRet(error, QString(), QString());
}

QString MultiMacroCommand::name() const
{
    return "multi";
}

QString MultiMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\multi{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *MultiMacroCommand::clone() const
{
    return new MultiMacroCommand(margs);
}

/*============================================================================
================================ ForMacroCommand =============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ForMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 5) ? new ForMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ForMacroCommand::ForMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ForMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    typedef bool(*PredicateI)(const int &, const int &);
    typedef bool(*PredicateF)(const double &, const double &);
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString s = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString lb = margs.at(1).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString ub = margs.at(2).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString step = margs.at(3).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    CONVERT2(lb, lb)
    CONVERT2(ub, ub)
    CONVERT2(step, step)
    if (blb != bub || blb != bstep || bub != bstep)
        return bRet(error, QString("Argument type mismatch"), QString("false"));
    if (blb)
    {
        PredicateF pred = (dlb < dub) ? &predLeqF : &predGeqF;
        if (!stack->set(s, QString::number(dlb, 'g', 15)))
            return bRet(error, QString("Failed to set variable"), QString("false"));
        for (double d = dlb; pred(d, dub); d += dstep)
        {
            for (int j = 4; j < margs.size(); ++j)
            {
                margs.at(j).toText(doc, stack, &err);
                if (!err.isEmpty())
                    return bRet(error, err, QString("false"));
            }
            if (!stack->set(s, QString::number(d + dstep, 'g', 15)))
                return bRet(error, QString("Failed to set variable"), QString("false"));
        }
    }
    else
    {
        PredicateI pred = (ilb < iub) ? &predLeqI : &predGeqI;
        if (!stack->set(s, QString::number(ilb)))
            return bRet(error, QString("Failed to set variable"), QString("false"));
        for (int i = ilb; pred(i, iub); i += istep)
        {
            for (int j = 4; j < margs.size(); ++j)
            {
                margs.at(j).toText(doc, stack, &err);
                if (!err.isEmpty())
                    return bRet(error, err, QString("false"));
                if (!stack->set(s, QString::number(i + istep)))
                    return bRet(error, QString("Failed to set variable"), QString("false"));
            }
        }
    }
    return bRet(error, QString(), QString("true"));
}

QString ForMacroCommand::name() const
{
    return "for";
}

QString ForMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\for";
    for (int i = 0; i < 5; ++i)
        s += "{" + margs.at(i).toText() + "}";
    for (int i = 5; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ForMacroCommand::clone() const
{
    return new ForMacroCommand(margs);
}

/*============================================================================
================================ IfMacroCommand ==============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *IfMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new IfMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

IfMacroCommand::IfMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString IfMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString err;
    QString c = margs.first().toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    bool b = false;
    if ("true" == c.toLower())
    {
        b = true;
    }
    else if ("false" != c.toLower())
    {
        bool ok = false;
        b = c.contains('.') ? (bool) c.toDouble(&ok) : (bool) c.toInt(&ok);
        if (!ok)
            return bRet(error, QString("Failed to convert"), QString());
    }
    if (!b && margs.size() < 3)
        return bRet(error, QString(), QString());
    QString s = margs.at(1 + (b ? 0 : 1)).toText(doc, stack, &err);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    return bRet(error, QString(), s);
}

QString IfMacroCommand::name() const
{
    return "if";
}

QString IfMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\if{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *IfMacroCommand::clone() const
{
    return new IfMacroCommand(margs);
}

/*============================================================================
================================ WaitMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *WaitMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1 && args.size() <= 3) ? new WaitMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

WaitMacroCommand::WaitMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString WaitMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString err;
    QString ns = margs.first().toText(doc, stack, &err);
    if (ns.isEmpty() || !err.isEmpty())
        return bRet(error, !err.isEmpty() ? err : QString("Invalid parameter"), QString("false"));
    int n = 0;
    err = toInt(ns, n);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    int k = BeQt::Second;
    bool b = false;
    bool kb = false;
    bool bb = false;
    for (int i = 1; i < margs.size(); ++i)
    {
        QString as = margs.at(i).toText(doc, stack, &err);
        if (!err.isEmpty())
            return bRet(error, err, QString("false"));
        if (!as.compare("ms", Qt::CaseInsensitive) || !as.compare("milliseconds", Qt::CaseInsensitive))
        {
            if (kb)
                return bRet(error, QString("Duplicate parameter"), QString("false"));
            k = 1;
            kb = true;
        }
        else if (!as.compare("s", Qt::CaseInsensitive) || !as.compare("seconds", Qt::CaseInsensitive))
        {
            if (kb)
                return "false";
            k = BeQt::Second;
            kb = true;
        }
        else if (!as.compare("m", Qt::CaseInsensitive) || !as.compare("minutes", Qt::CaseInsensitive))
        {
            if (kb)
                return "false";
            k = BeQt::Minute;
            kb = true;
        }
        else if (!as.compare("b", Qt::CaseInsensitive) || !as.compare("blocking", Qt::CaseInsensitive))
        {
            if (bb)
                return "false";
            b = true;
            bb = true;
        }
        else if (!as.compare("nb", Qt::CaseInsensitive) || !as.compare("non-blocking", Qt::CaseInsensitive))
        {
            if (bb)
                return "false";
            b = false;
            bb = true;
        }
        else
        {
            return bRet(error, QString("Unknown parameter"), QString("false"));
        }
    }
    if (b)
        ThreadHack::waitBlocking(k * n);
    else
        BeQt::waitNonBlocking(k * n);
    return bRet(error, QString(), QString("true"));
}

QString WaitMacroCommand::name() const
{
    return "wait";
}

QString WaitMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\wait{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *WaitMacroCommand::clone() const
{
    return new WaitMacroCommand(margs);
}

/*============================================================================
================================ FindMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *FindMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1 && args.size() <= 5) ? new FindMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

FindMacroCommand::FindMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString FindMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    bool w = false;
    bool bw = false;
    bool nc = false;
    QString what;
    QString err = getReplaceMndParameters(margs, doc, stack, what);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    err = getReplaceOptParameters(margs, doc, stack, 1, cs, &w, &bw, &nc);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    bool b = doc->findNext(what, createFindFlags(cs, w, bw), !nc);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString FindMacroCommand::name() const
{
    return "find";
}

QString FindMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\find{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *FindMacroCommand::clone() const
{
    return new FindMacroCommand(margs);
}

/*============================================================================
================================ ReplaceMacroCommand =========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ReplaceMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 6) ? new ReplaceMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ReplaceMacroCommand::ReplaceMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ReplaceMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack,
                                     QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    bool w = false;
    bool bw = false;
    bool nc = false;
    QString what;
    QString ntext;
    QString err = getReplaceMndParameters(margs, doc, stack, what, &ntext);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    err = getReplaceOptParameters(margs, doc, stack, 2, cs, &w, &bw, &nc);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    bool b = doc->findNext(what, createFindFlags(cs, w, bw), !nc);
    if (!b)
        bRet(error, QString(), QString("false"));
    b = doc->replaceNext(ntext);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString ReplaceMacroCommand::name() const
{
    return "replace";
}

QString ReplaceMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\replace{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ReplaceMacroCommand::clone() const
{
    return new ReplaceMacroCommand(margs);
}

/*============================================================================
================================ ReplaceSelMacroCommand ======================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ReplaceSelMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new ReplaceSelMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ReplaceSelMacroCommand::ReplaceSelMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ReplaceSelMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack,
                                        QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("-1"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    QString what;
    QString ntext;
    QString err = getReplaceMndParameters(margs, doc, stack, what, &ntext);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    err = getReplaceOptParameters(margs, doc, stack, 2, cs);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    int n = doc->replaceInSelection(what, ntext, cs);
    return bRet(error, QString(), QString::number(n));
}

QString ReplaceSelMacroCommand::name() const
{
    return "replaceSel";
}

QString ReplaceSelMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\replaceSel{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ReplaceSelMacroCommand::clone() const
{
    return new ReplaceSelMacroCommand(margs);
}

/*============================================================================
================================ ReplaceDocMacroCommand ======================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ReplaceDocMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2 && args.size() <= 3) ? new ReplaceDocMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ReplaceDocMacroCommand::ReplaceDocMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ReplaceDocMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack,
                                        QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("-1"));
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    QString what;
    QString ntext;
    QString err = getReplaceMndParameters(margs, doc, stack, what, &ntext);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    err = getReplaceOptParameters(margs, doc, stack, 2, cs);
    if (!err.isEmpty())
        return bRet(error, err, QString("-1"));
    int n = doc->replaceInDocument(what, ntext, cs);
    return bRet(error, QString(), QString::number(n));
}

QString ReplaceDocMacroCommand::name() const
{
    return "replaceDoc";
}

QString ReplaceDocMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\replaceDoc{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ReplaceDocMacroCommand::clone() const
{
    return new ReplaceDocMacroCommand(margs);
}

/*============================================================================
================================ ExecMacroCommand ============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1) ? new ExecMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecMacroCommand::ExecMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QStringList args;
    err = getCommandArgs(margs, 1, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString dir;
    getWorkingDir(doc, dir);
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    if (!dir.isEmpty())
        proc.setWorkingDirectory(dir);
    BeQt::startProcess(&proc, cmd, args);
    BeQt::waitNonBlocking(&proc, SIGNAL(started()), 30 * BeQt::Second);
    if (proc.state() != QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process start timeout"), QString());
    }
    BeQt::waitNonBlocking(&proc, SIGNAL(finished(int)), 5 * BeQt::Minute);
    if (proc.state() == QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process execution timeout"), QString());
    }
    return bRet(error, QString(), processOutputToText(proc.readAll()));
}

QString ExecMacroCommand::name() const
{
    return "exec";
}

QString ExecMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\exec{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecMacroCommand::clone() const
{
    return new ExecMacroCommand(margs);
}

/*============================================================================
================================ ExecFMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecFMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2) ? new ExecFMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecFMacroCommand::ExecFMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecFMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString());
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QString fn;
    QString dir;
    err = getFileName(margs.at(1), doc, stack, fn, &dir);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    QStringList args;
    err = getCommandArgs(margs, 2, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString());
    args.prepend(fn);
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.setWorkingDirectory(dir);
    BeQt::startProcess(&proc, cmd, args);
    BeQt::waitNonBlocking(&proc, SIGNAL(started()), 30 * BeQt::Second);
    if (proc.state() != QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process start timeout"), QString());
    }
    BeQt::waitNonBlocking(&proc, SIGNAL(finished(int)), 5 * BeQt::Minute);
    if (proc.state() == QProcess::Running)
    {
        proc.kill();
        return bRet(error, QString("Process execution timeout"), QString());
    }
    return bRet(error, QString(), processOutputToText(proc.readAll()));
}

QString ExecFMacroCommand::name() const
{
    return "execF";
}

QString ExecFMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\execF{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecFMacroCommand::clone() const
{
    return new ExecFMacroCommand(margs);
}

/*============================================================================
================================ ExecDMacroCommand ===========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecDMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1) ? new ExecDMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecDMacroCommand::ExecDMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecDMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QStringList args;
    err = getCommandArgs(margs, 1, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString dir;
    getWorkingDir(doc, dir);
    bool b = !dir.isEmpty() ? QProcess::startDetached(cmd, args, dir) : QProcess::startDetached(cmd, args);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString ExecDMacroCommand::name() const
{
    return "execD";
}

QString ExecDMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\execD{" + margs.first().toText() + "}";
    for (int i = 1; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecDMacroCommand::clone() const
{
    return new ExecDMacroCommand(margs);
}

/*============================================================================
================================ ExecFDMacroCommand ==========================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *ExecFDMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 2) ? new ExecFDMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

ExecFDMacroCommand::ExecFDMacroCommand(const QList<MacroCommandArgument> &args) :
    MultiArgMacroCommand(args)
{
    //
}

/*============================== Public methods ============================*/

QString ExecFDMacroCommand::execute(BAbstractCodeEditorDocument *doc, MacroExecutionStack *stack, QString *error) const
{
    if (!doc || !stack || !isValid())
        return bRet(error, QString("Internal error"), QString("false"));
    QString cmd;
    QString err = getCommand(margs.first(), doc, stack, cmd);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QString fn;
    QString dir;
    err = getFileName(margs.at(1), doc, stack, fn, &dir);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    QStringList args;
    err = getCommandArgs(margs, 2, doc, stack, args);
    if (!err.isEmpty())
        return bRet(error, err, QString("false"));
    args.prepend(fn);
    bool b = !dir.isEmpty() ? QProcess::startDetached(cmd, args, dir) : QProcess::startDetached(cmd, args);
    return bRet(error, QString(), QString(b ? "true" : "false"));
}

QString ExecFDMacroCommand::name() const
{
    return "execFD";
}

QString ExecFDMacroCommand::toText() const
{
    if (!isValid())
        return "";
    QString s = "\\execFD{" + margs.first().toText() + "}{" + margs.at(1).toText() + "}";
    for (int i = 2; i < margs.size(); ++i)
        s += "[" + margs.at(i).toText() + "]";
    return s;
}

AbstractMacroCommand *ExecFDMacroCommand::clone() const
{
    return new ExecFDMacroCommand(margs);
}
