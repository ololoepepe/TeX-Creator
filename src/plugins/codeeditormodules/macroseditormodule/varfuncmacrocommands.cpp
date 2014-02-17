#include "varfuncmacrocommands.h"
#include "macrocommand.h"
#include "macrocommandargument.h"
#include "macroexecutionstack.h"
#include "global.h"

#include <BeQtGlobal>
#include <BAbstractCodeEditorDocument>

#include <QList>
#include <QMap>
#include <QString>
#include <QFileInfo>

/*============================================================================
================================ Global static functions =====================
============================================================================*/

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
    AbstractMacroCommand(args)
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
    err = Global::formatText(s, f);
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
    AbstractMacroCommand(args)
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
    AbstractMacroCommand(args)
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
    AbstractMacroCommand(args)
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
    AbstractMacroCommand(args)
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
    AbstractMacroCommand(args)
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
    err = Global::formatText(v, f);
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
    AbstractMacroCommand(args)
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
    AbstractMacroCommand(args)
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
    err = Global::formatText(v, f);
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
    AbstractMacroCommand(args)
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
    AbstractMacroCommand(args)
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
    err = Global::formatText(v, f);
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
================================ CMacroCommand ===============================
============================================================================*/

/*============================== Static public methods =====================*/

AbstractMacroCommand *CMacroCommand::create(const QList<MacroCommandArgument> &args)
{
    return (args.size() >= 1 && args.size() <= 2) ? new CMacroCommand(args) : 0;
}

/*============================== Private constructors ======================*/

CMacroCommand::CMacroCommand(const QList<MacroCommandArgument> &args) :
    AbstractMacroCommand(args)
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
    err = Global::formatText(v, f);
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
