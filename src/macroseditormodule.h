#ifndef MACROSEDITORMODULE_H
#define MACROSEDITORMODULE_H

class MacroCommand;

class BAbstractCodeEditorDocument;
class BSignalDelayProxy;

class QEvent;
class QListWidgetItem;
class QByteArray;

#include <BAbstractEditorModule>
#include <BCodeEditor>

#include <QObject>
#include <QList>
#include <QPointer>
#include <QAction>
#include <QString>
#include <QSplitter>
#include <QListWidget>

/*============================================================================
================================ MacroCommand ================================
============================================================================*/

class MacroCommand
{
public:
    explicit MacroCommand(const QString &t = QString());
    explicit MacroCommand(const QEvent *e);
    MacroCommand(const MacroCommand &other);
public:
    void clear();
    void execute(BAbstractCodeEditorDocument *doc) const;
    bool fromText(const QString &t);
    bool fromKeyPress(const QEvent *e);
    QString toText() const;
    bool isValid() const;
public:
    MacroCommand &operator =(const MacroCommand &other);
private:
    void init();
private:
    bool keypress;
    QString command;
};

/*============================================================================
================================ Macro =======================================
============================================================================*/

class Macro
{
public:
    explicit Macro(const QString &fileName = QString());
    Macro(const Macro &other);
public:
    void clear();
    void execute(BAbstractCodeEditorDocument *doc, BCodeEditor *cedtr) const;
    bool recordKeyPress(const QEvent *e, QString *s = 0);
    bool fromText(const QString &text);
    bool fromFile(const QString &fileName);
    QString toText() const;
    bool toFile(const QString &fileName) const;
    bool isValid() const;
public:
    Macro &operator=(const Macro &other);
private:
    QList<MacroCommand> mcommands;
};

/*============================================================================
================================ MacrosEditorModule ==========================
============================================================================*/

class MacrosEditorModule : public BAbstractEditorModule
{
    Q_OBJECT
public:
    enum Action
    {
        StartStopRecordingAction,
        ClearAction,
        PlayAction,
        ShowHideAction,
        LoadAction,
        SaveAsAction,
        OpenUserMacrosDirAction
    };
public:
    explicit MacrosEditorModule(QObject *parent = 0);
    ~MacrosEditorModule();
public:
    QString id() const;
    QAction *action(int type);
    QList<QAction *> actions(bool extended = false);
    bool eventFilter(QObject *o, QEvent *e);
    QByteArray saveState() const;
    void restoreState(const QByteArray &state);
    bool isPlaying() const;
public slots:
    void startStopRecording();
    void clearMacro();
    void playMacro(int n = 0);
    void playMacro5();
    void playMacro10();
    void playMacro20();
    void playMacro50();
    void playMacro100();
    void playMacroN();
    void showHideMacrosConsole();
    bool loadMacro(const QString &fileName = QString());
    bool saveMacroAs();
    void openUserDir();
    void reloadMacros();
protected:
    void editorSet(BCodeEditor *edr);
    void editorUnset(BCodeEditor *edr);
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
private:
    static QString fileDialogFilter();
private:
    void resetStartStopAction();
    void resetShowHideAction();
    void checkActions();
    void appendPtedtText(const QString &text);
    void setPtedtText(const QString &text);
    void clearPtedt();
private slots:
    void retranslateUi();
    void ptedtTextChanged();
    void lstwgtCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void cedtrCurrentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void cedtrDocumentAboutToBeAdded(BAbstractCodeEditorDocument *doc);
    void cedtrDocumentAboutToBeRemoved(BAbstractCodeEditorDocument *doc);
    void cedtrCurrentDocumentFileNameChanged(const QString &fileName);
private:
    Macro mmacro;
    bool mplaying;
    bool mrecording;
    BAbstractCodeEditorDocument *mprevDoc;
    //
    QPointer<QAction> mactStartStop;
    QPointer<QAction> mactClear;
    QPointer<QAction> mactPlay;
    QPointer<QAction> mactPlay5;
    QPointer<QAction> mactPlay10;
    QPointer<QAction> mactPlay20;
    QPointer<QAction> mactPlay50;
    QPointer<QAction> mactPlay100;
    QPointer<QAction> mactPlayN;
    QPointer<QAction> mactShowHide;
    QPointer<QAction> mactLoad;
    QPointer<QAction> mactSaveAs;
    QPointer<QAction> mactOpenDir;
    QPointer<BCodeEditor> mcedtr;
    QPointer<QListWidget> mlstwgt;
    QPointer<QSplitter> mspltr;
    BSignalDelayProxy *mproxy;
private:
    Q_DISABLE_COPY(MacrosEditorModule)
};

#endif // MACROSEDITORMODULE_H
