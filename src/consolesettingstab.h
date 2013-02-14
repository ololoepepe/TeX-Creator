#ifndef CONSOLESETTINGSTAB_H
#define CONSOLESETTINGSTAB_H

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QString;
class QIcon;

#include <BAbstractSettingsTab>

/*============================================================================
================================ ConsoleSettingsTab ==========================
============================================================================*/

class ConsoleSettingsTab : public BAbstractSettingsTab
{
    Q_OBJECT
public:
    explicit ConsoleSettingsTab();
public:
    static bool hasFallbackToLocalCompiler();
    static bool getUseRemoteCompiler();
    static QString getCompilerName();
    static QStringList getCompilerOptions();
    static QString getCompilerOptionsString();
    static QStringList getCompilerCommands();
    static QString getCompilerCommandsString();
    static bool getMakeindexEnabled();
    static bool getDvipsEnabled();
    static bool getAlwaysLatinEnabled();
    static bool getFallbackToLocalCompiler();
    static void setUseRemoteCompiler(bool b);
    static void setCompilerName(const QString &name);
    static void setCompilerOptions(const QStringList &list);
    static void setCompilerOptions(const QString &string);
    static void setCompilerCommands(const QStringList &list);
    static void setCompilerCommands(const QString &string);
    static void setMakeindexEnabled(bool enabled);
    static void setDvipsEnabled(bool enabled);
    static void setAlwaysLatinEnabled(bool enabled);
    static void setFallbackToLocalCompiler(bool b);
public:
    QString title() const;
    QIcon icon() const;
    bool hasAdvancedMode() const;
    void setAdvancedMode(bool enabled);
    bool restoreDefault();
    bool saveSettings();
private:
    static QStringList splitArguments(const QString &string);
    static QString joinArguments(const QStringList &list);
private:
    QCheckBox *mcboxRemoteCompiler;
    QCheckBox *mcboxFallbackToLocalCompiler;
    QComboBox *mcmboxName;
    QLineEdit *mledtOptions;
    QLineEdit *mledtCommands;
    QCheckBox *mcboxMakeindex;
    QCheckBox *mcboxDvips;
    QCheckBox *mcboxAlwaysLatin;
private:
    Q_DISABLE_COPY(ConsoleSettingsTab)
};

#endif // CONSOLESETTINGSTAB_H
