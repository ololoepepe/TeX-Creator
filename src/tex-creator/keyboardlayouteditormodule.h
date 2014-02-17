#ifndef KEYBOARDLAYOUTEDITORMODULE_H
#define KEYBOARDLAYOUTEDITORMODULE_H

class BCodeEditor;
class BAbstractCodeEditorDocument;

class QString;

#include <BAbstractEditorModule>

#include <QObject>
#include <QList>
#include <QPointer>
#include <QAction>
#include <QMap>
#include <QChar>

/*============================================================================
================================ KeyboardLayoutMap ===========================
============================================================================*/

class KeyboardLayoutMap
{
public:
    explicit KeyboardLayoutMap();
    KeyboardLayoutMap(const KeyboardLayoutMap &other);
    explicit KeyboardLayoutMap(const QString &fileName);
public:
    bool load(const QString &fileName);
    bool isValid() const;
    bool switchLayout(QString &text) const;
public:
    KeyboardLayoutMap &operator=(const KeyboardLayoutMap &other);
private:
    QMap<QChar, QChar> mdirect;
    QMap<QChar, QChar> mreverse;
    QList<QChar> mdirectUnique;
    QList<QChar> mreverseUnique;
};

/*============================================================================
================================ KeyboardLayoutEditorModule ==================
============================================================================*/

class KeyboardLayoutEditorModule : public BAbstractEditorModule
{
    Q_OBJECT
public:
    enum Action
    {
        SwitchSelectedTextLayoutAction,
        OpenUserKLMDirAction
    };
public:
    explicit KeyboardLayoutEditorModule(QObject *parent = 0);
public:
    QString id() const;
    QAction *action(int type);
    QList<QAction *> actions(bool extended = false);
public slots:
    void switchLayout();
    void reloadMap();
    void openUserDir();
protected:
    void currentDocumentChanged(BAbstractCodeEditorDocument *doc);
    void documentHasSelectionChanged(bool hasSelection);
private:
    void checkSwitchAction();
private slots:
    void retranslateUi();
private:
    QPointer<QAction> mactSwitch;
    QPointer<QAction> mactOpenDir;
    KeyboardLayoutMap mmap;
private:
    Q_DISABLE_COPY(KeyboardLayoutEditorModule)
};

#endif // KEYBOARDLAYOUTEDITORMODULE_H
