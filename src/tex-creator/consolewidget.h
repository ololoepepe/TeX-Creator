#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

class BTerminalWidget;
class BCodeEditor;
class BAbstractCodeEditorDocument;

class QToolBar;
class QAction;
class QEvent;
class QSignalMapper;

#include <QWidget>
#include <QString>
#include <QTextCharFormat>
#include <QMap>
#include <QList>
#include <QStringList>

/*============================================================================
================================ ConsoleWidget ===============================
============================================================================*/

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
        OpenPsAction,
        SwitchCompilerAction,
        SettingsAction
    };
public:
    explicit ConsoleWidget(BCodeEditor *cedtr, QWidget *parent = 0);
public:
    bool eventFilter(QObject *object, QEvent *event);
    QAction *consoleAction(Action actId) const;
    QList<QAction *> consoleActions(bool withSeparators = false) const;
private:
    static QString fileNameNoSuffix(const QString &fileName);
private:
    void initKeyMap();
    void initGui();
    QAction *createAction(int id, const QString &iconFileName = QString(), const QString &shortcut = QString(),
                          bool enabled = false);
    void compile(bool op = false);
    void open(bool pdf = true);
    void start( const QString &command, const QStringList &args = QStringList() );
    void start(const QString &command, const QString &arg);
    void noFileNameError();
    void showSettings();
    void setUiEnabled(bool b);
private slots:
    void retranslateUi();
    void performAction(int actId);
    void checkActions(BAbstractCodeEditorDocument *doc);
    void finished(int exitCode);
    void updateSwitchCompilerAction();
private:
    QMap<int, int> mkeyMap;
    QMap<int, QAction *> mactMap;
    QSignalMapper *mmprActions;
    BCodeEditor *mcedtr;
    QString mfileName;
    QString mcommand;
    bool mmakeindex;
    bool mdvips;
    bool mopen;
    bool mremote;
    //
    //vlt
      QToolBar *mtbar;
        //actions
      BTerminalWidget *mtermwgt;
private:
    Q_DISABLE_COPY(ConsoleWidget)
};

#endif // CONSOLEWIDGET_H