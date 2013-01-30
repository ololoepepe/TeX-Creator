#ifndef MAINDOCUMENTEDITORMODULE_H
#define MAINDOCUMENTEDITORMODULE_H

class BCodeEditor;
class BCodeEditorDocument;

class QString;

#include <BAbstractEditorModule>

#include <QObject>
#include <QList>
#include <QPointer>
#include <QAction>

/*============================================================================
================================ MainDocumentEditorModule ====================
============================================================================*/

class MainDocumentEditorModule : public BAbstractEditorModule
{
    Q_OBJECT
public:
    enum Action
    {
        SwitchCurrentDocumentMainAction
    };
public:
    explicit MainDocumentEditorModule(QObject *parent = 0);
    ~MainDocumentEditorModule();
public:
    QString id() const;
    QAction *action(int type);
    QList<QAction *> actions(bool extended = false);
    void setMainDocument(BCodeEditorDocument *doc);
    bool isCurrentDocumentMain() const;
    BCodeEditorDocument *mainDocument() const;
    QString mainDocumentFileName() const;
public slots:
    void switchCurrentDocumentMain();
protected:
    void editorSet(BCodeEditor *edr);
    void editorUnset(BCodeEditor *edr);
    void currentDocumentChanged(BCodeEditorDocument *doc);
private:
    void resetAction();
private slots:
    void retranslateUi();
signals:
    void mainDocumentChanged(BCodeEditorDocument *doc);
private:
    BCodeEditorDocument *mmainDocument;
    QPointer<QAction> mact;
private:
    Q_DISABLE_COPY(MainDocumentEditorModule)
};

#endif // MAINDOCUMENTEDITORMODULE_H
