#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

class BTerminal;
class BTextEditor;

class QString;
class QProcess;
class QVBoxLayout;
class QToolBar;
class QAction;
class QLabel;
class QComboBox;
class QLineEdit;
class QEvent;
class QSignalMapper;
class QCheckBox;

#include <QWidget>
#include <QTextCharFormat>
#include <QMap>
#include <QList>

class ConsoleWidget : public QWidget
{
    Q_OBJECT
public:
    enum Action
    {
        ClearAction,
        CompileAction,
        CompileAndOpenAction,
        OpenPdfAction,
        OpenPsAction
    };
    //
    explicit ConsoleWidget(const QString &settingsGroup, QWidget *parent = 0);
    //
    bool eventFilter(QObject *object, QEvent *event);
    void setTextEditor(BTextEditor *editor);
    void saveSettings();
    QAction *consoleAction(Action actId) const;
    QList<QAction *> consoleActions() const;
public slots:
    void performAction(int actId);
private:
    const QString mCSettingsGroup;
    //
    QMap<int, int> mkeyMap;
    BTerminal *mTerminal;
    BTextEditor *mEditor;
    QString mFileName;
    QString mWorkingDir;
    bool mRun;
    QTextCharFormat mTcfExp;
    QTextCharFormat mTcfExpB;
    QTextCharFormat mTcfExpI;
    QTextCharFormat mTcfErr;
    QTextCharFormat mTcfErrB;
    QTextCharFormat mTcfErrI;
    QMap<int, QAction *> mActMap;
    QSignalMapper *mmprActions;
    //
    QVBoxLayout *mvlt;
      QToolBar *mtbar;
        //actions
        //separator
        QLabel *mlblCommand;
        QComboBox *mcmboxCommand;
        //separator
        QCheckBox *mcboxMakeindex;
        QCheckBox *mcboxDvips;
        //separator
        QLabel *mlblParameters;
        QLineEdit *mledtParameters;
        //separator
        QCheckBox *mcboxAlwaysLatin;
      //edit
    //
    void loadSettings();
    void initKeyMap();
    void initGui();
    QAction *createAction(int id, const QString &iconFileName, const QString &shortcut, bool enabled = false);
    void compile(bool run = false);
    void open(bool pdf = true);
    void startCompiler();
    void startMakeindex();
    void startDvips();
    void executingMessage(const QString &program);
    void failedMessage(const QString &program);
    void finishedMessage(const QString &program, int code);
private slots:
    void retranslateUi();
    void checkCompileAvailable();
    void finished(int exitCode);
};

#endif // CONSOLEWIDGET_H
