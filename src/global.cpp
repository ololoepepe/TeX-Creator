#include "global.h"
#include "application.h"

#include <BCodeEdit>
#include <BCodeEditor>
#include <BPasswordWidget>

#include <QFont>
#include <QString>
#include <QTextCodec>
#include <QStringList>
#include <QByteArray>
#include <QVariant>
#include <QSettings>

namespace Global
{

QStringList splitArguments(const QString &string)
{
    QStringList list = string.split(' ', QString::SkipEmptyParts);
    if (list.isEmpty())
        return list;
    int x = -1;
    foreach (int i, bRangeR(list.size() - 1, 0))
    {
        QString &arg = list[i];
        if (x < 0 && arg.at(arg.length() - 1) == '\"')
            x = i;
        if (x >= 0 && arg.at(0) == '\"')
        {
            if (x != i)
                foreach (int j, bRange(x, i + 1))
                    arg.append(" " + list.takeAt(j));
            x = -1;
        }
    }
    return list;
}

QString joinArguments(const QStringList &list)
{
    QStringList nlist = list;
    if ( !nlist.isEmpty() )
    {
        foreach (int i, bRangeD(0, list.size() - 1))
        {
            QString &arg = nlist[i];
            if (arg.contains(' ') && (arg.at(0) != '\"' || arg.at(arg.length() - 1) != '\"'))
                arg.prepend('\"').append('\"');
        }
    }
    return nlist.join(" ");
}

//CodeEditor

void setEditFont(const QFont &font)
{
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setEditFont(font);
    setEditFontFamily(font.family());
    setEditFontPointSize(font.pointSize());
}

void setEditFontFamily(const QString &family)
{
    if (family.isEmpty())
        return;
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setEditFontFamily(family);
    bSettings->setValue("CodeEditor/edit_font_family", family);
}

void setEditFontPointSize(int pointSize)
{
    if (pointSize < 1)
        return;
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setEditFontPointSize(pointSize);
    bSettings->setValue("CodeEditor/edit_font_point_size", pointSize);
}

void setDefaultCodec(QTextCodec *codec)
{
    if (!codec)
        return;
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setDefaultCodec(codec);
    bSettings->setValue("CodeEditor/default_codec", BCodeEditor::codecName(codec));
}

void setDefaultCodec(const QByteArray &codecName)
{
    setDefaultCodec(QTextCodec::codecForName(codecName));
}

void setDefaultCodec(const QString &codecName)
{
    setDefaultCodec(codecName.toLatin1());
}

void setEditLineLength(int lineLength)
{
    if (lineLength < 10 || lineLength > 1000)
        return;
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setEditLineLength(lineLength);
    bSettings->setValue("CodeEditor/edit_line_length", lineLength);
}

void setEditTabWidth(int tabWidth)
{
    switch (tabWidth)
    {
    case BCodeEdit::TabWidth2:
    case BCodeEdit::TabWidth4:
    case BCodeEdit::TabWidth8:
        break;
    default:
        return;
    }
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setEditTabWidth(static_cast<BCodeEdit::TabWidth>(tabWidth));
    bSettings->setValue("CodeEditor/edit_tab_width", tabWidth);
}

void setFileHistory(const QStringList &history)
{
    foreach (BCodeEditor *edr, Application::codeEditors())
        edr->setFileHistory(history);
    bSettings->setValue("CodeEditor/file_history", history);
}

void setDocumentDriverState(const QByteArray &state)
{
    bSettings->setValue("CodeEditor/document_driver_state", state);
}

void setSearchModuleState(const QByteArray &state)
{
    bSettings->setValue("CodeEditor/search_moudle_state", state);
}

//Console

void setCompilerName(const QString &command)
{
    if (command.isEmpty())
        return;
    bSettings->setValue("Console/compiler_name", command);
}

void setCompilerOptions(const QStringList &list)
{
    bSettings->setValue("Console/compiler_options", list);
}

void setCompilerOptions(const QString &string)
{
    setCompilerOptions(splitArguments(string));
}

void setCompilerCommands(const QStringList &list)
{
    bSettings->setValue("Console/compiler_commands", list);
}

void setCompilerCommands(const QString &string)
{
    setCompilerCommands(splitArguments(string));
}

void setMakeindexEnabled(bool enabled)
{
    bSettings->setValue("Console/makeindex_enabled", enabled);
}

void setDvipsEnabled(bool enabled)
{
    bSettings->setValue("Console/dvips_enabled", enabled);
}

void setUseRemoteCompiler(bool b)
{
    bSettings->setValue("Console/use_remote_compiler", b);
}

void setFallbackToLocalCompiler(bool b)
{
    bSettings->setValue("Console/fallback_to_local_compiler", b);
}

void setAlwaysLatinEnabled(bool enabled)
{
    bSettings->setValue("Console/always_latin_enabled", enabled);
}

//General

void setMultipleWindowsEnabled(bool enabled)
{
    bSettings->setValue("Core/multiple_windows_enabled", enabled);
}

//TeXSample

void setAutoconnection(bool enabled)
{
    bSettings->setValue("TeXSample/Client/autoconnection", enabled);
}

void setHost(const QString &host)
{
    bSettings->setValue("TeXSample/Client/host", host);
}

void setHostHistory(const QStringList &history)
{
    bSettings->setValue("TeXSample/Client/host_history", history);
}

void setLogin(const QString &login)
{
    bSettings->setValue("TeXSample/Client/login", login);
}

void setPasswordSate(const QByteArray &state)
{
    bSettings->setValue("TeXSample/Client/password_state", state);
}

void setPassword(const QByteArray &pwd)
{
    BPasswordWidget::PasswordWidgetData data = BPasswordWidget::stateToData(passwordState());
    data.encryptedPassword = pwd;
    data.charCount = -1;
    data.password.clear();
    setPasswordSate(BPasswordWidget::dataToState(data));
}

void setPassword(const QString &pwd)
{
    BPasswordWidget::PasswordWidgetData data = BPasswordWidget::stateToData(passwordState());
    data.password = pwd;
    data.charCount = -1;
    data.encryptedPassword.clear();
    setPasswordSate(BPasswordWidget::dataToState(data));
}

void setCachingEnabled(bool enabled)
{
    bSettings->setValue("TeXSample/Cache/enabled", enabled);
}

//CodeEditor

QFont editFont()
{
    QFont fnt = Application::createMonospaceFont();
    fnt.setFamily(bSettings->value("CodeEditor/edit_font_family", fnt.family()).toString());
    fnt.setPointSize(bSettings->value("CodeEditor/edit_font_point_size", fnt.pointSize()).toInt());
    return fnt;
}

QString editFontFamily()
{
    return editFont().family();
}

int editFontPointSize()
{
    return editFont().pointSize();
}

QTextCodec *defaultCodec()
{
    return QTextCodec::codecForName(bSettings->value("CodeEditor/default_codec",
                                                     QTextCodec::codecForLocale()->name()).toByteArray());
}

QString defaultCodecName()
{
    return BCodeEditor::codecName(defaultCodec());
}

int editLineLength()
{
    return bSettings->value("CodeEditor/edit_line_length", 120).toInt();
}

BCodeEdit::TabWidth editTabWidth()
{
    return static_cast<BCodeEdit::TabWidth>(bSettings->value("CodeEditor/edit_tab_width",
                                                             BCodeEdit::TabWidth4).toInt());
}

QStringList fileHistory()
{
    return bSettings->value("CodeEditor/file_history").toStringList();
}

QByteArray documentDriverState()
{
    return bSettings->value("CodeEditor/document_driver_state").toByteArray();
}

QByteArray searchModuleState()
{
    return bSettings->value("CodeEditor/search_moudle_state").toByteArray();
}

//Console

bool hasFallbackToLocalCompiler()
{
    return bSettings->contains("Console/fallback_to_local_compiler");
}

QString compilerName()
{
    return bSettings->value("Console/compiler_name", "pdflatex").toString();
}

QStringList compilerOptions()
{
    return bSettings->value("Console/compiler_options").toStringList();
}

QString compilerOptionsString()
{
    return joinArguments(compilerOptions());
}

QStringList compilerCommands()
{
    return bSettings->value("Console/compiler_commands").toStringList();
}

QString compilerCommandsString()
{
    return joinArguments(compilerCommands());
}

bool makeindexEnabled()
{
    return bSettings->value("Console/makeindex_enabled", false).toBool();
}

bool dvipsEnabled()
{
    return bSettings->value("Console/dvips_enabled", false).toBool();
}

bool useRemoteCompiler()
{
    return bSettings->value("Console/use_remote_compiler").toBool();
}

bool fallbackToLocalCompiler()
{
    return bSettings->value("Console/fallback_to_local_compiler").toBool();
}

bool alwaysLatinEnabled()
{
    return bSettings->value("Console/always_latin_enabled", false).toBool();
}

//General

bool multipleWindowsEnabled()
{
    return bSettings->value("Core/multiple_windows_enabled", false).toBool();
}

//TeXSample

bool hasTexsample()
{
    return bSettings->contains("TeXSample/Client/autoconnection");
}

bool autoconnection()
{
    return bSettings->value("TeXSample/Client/autoconnection", true).toBool();
}

QString host()
{
    QString host = bSettings->value("TeXSample/Client/host", "auto_select").toString();
    return hostHistory().contains(host) ? host : "auto_select";
}

QStringList hostHistory()
{
    return bSettings->value("TeXSample/Client/host_history").toStringList();
}

QString login()
{
    return bSettings->value("TeXSample/Client/login").toString();
}

QByteArray passwordState()
{
    return bSettings->value("TeXSample/Client/password_state").toByteArray();
}

QByteArray password()
{
    return BPasswordWidget::stateToData(passwordState()).encryptedPassword;
}

bool cachingEnabled()
{
    return bSettings->value("TeXSample/Cache/enabled", true).toBool();
}

}
