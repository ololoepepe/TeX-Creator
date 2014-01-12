#ifndef GLOBAL_H
#define GLOBAL_H

#include <BCodeEdit>
#include <BCodeEditor>

#include <TCompilerParameters>

class BPassword;

class QFont;
class QString;
class QTextCodec;
class QStirngList;
class QByteArray;

namespace Global
{

//CodeEditor
void setEditorDocumentType(int t);
void setEditorSpellCheckEnabled(bool b);
void setEditFont(const QFont &font);
void setEditFontFamily(const QString &family);
void setEditFontPointSize(int pointSize);
void setAutoCodecDetectionEnabled(bool b);
void setDefaultCodec(QTextCodec *codec);
void setDefaultCodec(const QByteArray &codecName);
void setDefaultCodec(const QString &codecName);
void setEditLineLength(int lineLength);
void setEditTabWidth(int tabWidth);
void setFileHistory(const QStringList &history);
void setDocumentDriverState(const QByteArray &state);
void setSearchModuleState(const QByteArray &state);
//Console
void setCompilerParameters(const TCompilerParameters &param);
void setCompiler(int c);
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
void setPasswordWidgetSate(const QByteArray &state);
void setPasswordState(const QByteArray &state);
void setPassword(const BPassword &pwd);
void setPassword(const QByteArray &pwd, int charCountHint = 0);
void setPassword(const QString &pwd);
void setCachingEnabled(bool enabled);
//CodeEditor
BCodeEditor::StandardDocumentType editorDocumentType();
bool editorSpellCheckEnabled();
QFont editFont();
QString editFontFamily();
int editFontPointSize();
bool autoCodecDetectionEnabled();
QTextCodec *defaultCodec();
QString defaultCodecName();
int editLineLength();
BeQt::TabWidth editTabWidth();
QStringList fileHistory();
QByteArray documentDriverState();
QByteArray searchModuleState();
//Console
bool hasFallbackToLocalCompiler();
TCompilerParameters compilerParameters();
TCompilerParameters::Compiler compiler();
QStringList compilerOptions();
QString compilerOptionsString(bool command = true);
QStringList compilerCommands();
QString compilerCommandsString(bool command = true);
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
QByteArray passwordWidgetState();
QByteArray passwordState();
BPassword password();
QByteArray encryptedPassword(int *charCountHint = 0);
bool cachingEnabled();
void savePasswordState();
void loadPasswordState();

}

#endif // GLOBAL_H
