#ifndef MACROSEDITORMODULE_H
#define MACROSEDITORMODULE_H

class BCodeEditor;
class BCodeEditorDocument;

class QEvent;

#include <BAbstractEditorModule>

#include <QObject>
#include <QList>
#include <QPointer>
#include <QAction>
#include <QString>
#include <QPlainTextEdit>

/*============================================================================
================================ Macro =======================================
============================================================================*/

class Macro
{
public:
    explicit Macro();
    Macro(const Macro &other);
    explicit Macro(const QString &fileName);
public:
    bool load(const QString &fileName);
    bool save(const QString &fileName) const;
    bool isValid() const;
    void clear();
    bool recordKeyPress(QEvent *e, QString *s = 0);
    void apply(QObject *object) const;
    QString toText() const;
public:
    Macro &operator=(const Macro &other);
private:
    struct KeyPress
    {
        int key;
        Qt::KeyboardModifiers modifiers;
        QString text;
        //
        KeyPress()
        {
            key = 0;
            modifiers = Qt::NoModifier;
        }
    };
private:
    static QString keyPressToText(const KeyPress &k);
private:
    QList<KeyPress> mkeys;
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
    bool isPlaying() const;
public slots:
    void startStopRecording();
    void clearMacro();
    void playMacro();
    void showHideMacrosConsole();
    bool loadMacro( const QString &fileName = QString() );
    bool saveMacroAs( const QString &fileName = QString() );
    void openUserDir();
protected:
    void editorSet(BCodeEditor *edr);
    void editorUnset(BCodeEditor *edr);
    void currentDocumentChanged(BCodeEditorDocument *doc);
private:
    static QString fileDialogFilter();
private:
    void resetStartStopAction();
    void resetShowHideAction();
    void checkActions();
private slots:
    void retranslateUi();
private:
    Macro mmacro;
    bool mplaying;
    bool mrecording;
    BCodeEditorDocument *mprevDoc;
    //
    QPointer<QAction> mactStartStop;
    QPointer<QAction> mactClear;
    QPointer<QAction> mactPlay;
    QPointer<QAction> mactShowHide;
    QPointer<QAction> mactLoad;
    QPointer<QAction> mactSaveAs;
    QPointer<QAction> mactOpenDir;
    QPointer<QPlainTextEdit> mptedt;
private:
    Q_DISABLE_COPY(MacrosEditorModule)
};

#endif // MACROSEDITORMODULE_H
