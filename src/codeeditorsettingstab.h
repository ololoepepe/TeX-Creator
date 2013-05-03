#ifndef CODEEDITORSETTINGSTAB_H
#define CODEEDITORSETTINGSTAB_H

class QString;
class QIcon;
class QFontComboBox;
class QSpinBox;
class QComboBox;
class QFont;
class QTextCodec;
class QStringList;
class QByteArray;

#include <BAbstractSettingsTab>
#include <BCodeEdit>

#include <QObject>
#include <QVariantMap>

/*============================================================================
================================ CodeEditorSettingsTab =======================
============================================================================*/

class CodeEditorSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit CodeEditorSettingsTab();
    ~CodeEditorSettingsTab();
public:
    static QFont getEditFont();
    static QString getEditFontFamily();
    static int getEditFontPointSize();
    static QTextCodec *getDefaultCodec();
    static QString getDefaultCodecName();
    static int getEditLineLength();
    static BCodeEdit::TabWidth getEditTabWidth();
    static QStringList getFileHistory();
    static QByteArray getDocumentDriverState();
    static QByteArray getSearchModuleState();
    static void setEditFont(const QFont &font);
    static void setEditFontFamily(const QString &family);
    static void setEditFontPointSize(int pointSize);
    static void setDefaultCodec(QTextCodec *codec);
    static void setDefaultCodec(const QByteArray &codecName);
    static void setDefaultCodec(const QString &codecName);
    static void setEditLineLength(int lineLength);
    static void setEditTabWidth(int tabWidth);
    static void setFileHistory(const QStringList &history);
    static void setDocumentDriverState(const QByteArray &state);
    static void setSearchModuleState(const QByteArray &state);
public:
    QString title() const;
    QIcon icon() const;
    bool restoreDefault();
    bool saveSettings();
private:
    QFontComboBox *mfntcmbox;
    QSpinBox *msboxFontPointSize;
    QSpinBox *msboxLineLength;
    QComboBox *mcmboxTabWidth;
    QComboBox *mcmboxEncoding;
private:
    Q_DISABLE_COPY(CodeEditorSettingsTab)
};

#endif // CODEEDITORSETTINGSTAB_H
