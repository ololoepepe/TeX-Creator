#ifndef GLOBAL_H
#define GLOBAL_H

#include <BCodeEdit>

class QFont;
class QString;
class QTextCodec;
class QStirngList;
class QByteArray;

namespace Global
{

//CodeEditor
void setEditFont(const QFont &font);
void setEditFontFamily(const QString &family);
void setEditFontPointSize(int pointSize);
void setDefaultCodec(QTextCodec *codec);
void setDefaultCodec(const QByteArray &codecName);
void setDefaultCodec(const QString &codecName);
void setEditLineLength(int lineLength);
void setEditTabWidth(int tabWidth);
void setFileHistory(const QStringList &history);
void setDocumentDriverState(const QByteArray &state);
void setSearchModuleState(const QByteArray &state);
//Console
void setCompilerName(const QString &name);
void setCompilerOptions(const QStringList &list);
void setCompilerOptions(const QString &string);
void setCompilerCommands(const QStringList &list);
void setCompilerCommands(const QString &string);
void setMakeindexEnabled(bool enabled);
void setDvipsEnabled(bool enabled);
void setUseRemoteCompiler(bool b);
void setFallbackToLocalCompiler(bool b);
void setAlwaysLatinEnabled(bool enabled);
//General
void setMultipleWindowsEnabled(bool enabled);
//TeXSample
void setAutoconnection(bool enabled);
void setHost(const QString &host);
void setHostHistory(const QStringList &history);
void setLogin(const QString &login);
void setPasswordSate(const QByteArray &state);
void setCachingEnabled(bool enabled);
//CodeEditor
QFont editFont();
QString editFontFamily();
int editFontPointSize();
QTextCodec *defaultCodec();
QString defaultCodecName();
int editLineLength();
BCodeEdit::TabWidth editTabWidth();
QStringList fileHistory();
QByteArray documentDriverState();
QByteArray searchModuleState();
//Console
bool hasFallbackToLocalCompiler();
QString compilerName();
QStringList compilerOptions();
QString compilerOptionsString();
QStringList compilerCommands();
QString compilerCommandsString();
bool makeindexEnabled();
bool dvipsEnabled();
bool useRemoteCompiler();
bool fallbackToLocalCompiler();
bool alwaysLatinEnabled();
//General
bool multipleWindowsEnabled();
//TeXSample
bool hasTexsample();
bool autoconnection();
QString host();
QStringList hostHistory();
QString login();
QByteArray passwordState();
QByteArray password();
bool cachingEnabled();

}

#endif // GLOBAL_H
