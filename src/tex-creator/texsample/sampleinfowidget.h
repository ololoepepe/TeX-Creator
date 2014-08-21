/****************************************************************************
**
** Copyright (C) 2012-2014 Andrey Bogdanov
**
** This file is part of TeX Creator.
**
** TeX Creator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** TeX Creator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with TeX Creator.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef SAMPLEINFOWIDGET_H
#define SAMPLEINFOWIDGET_H

class SampleModel;

class TAbstractCache;
class TAuthorInfoList;
class TListWidget;
class TNetworkClient;
class TTagWidget;

class BCodeEditor;
class BInputField;

class QByteArray;
class QCheckBox;
class QComboBox;
class QFormLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QTextCodec;
class QToolButton;
class QVariant;
class QVBoxLayout;

#include <TTexProject>

#include <QString>
#include <QWidget>

/*============================================================================
================================ SampleInfoWidget ============================
============================================================================*/

class SampleInfoWidget : public QWidget
{
    Q_OBJECT
public:
    enum Mode
    {
        AddMode,
        EditAdminMode,
        EditSelfMode,
        ShowMode
    };
private:
    static const QString DateTimeFormat;
    static const Qt::TextInteractionFlags TextInteractionFlags;
private:
    const Mode mmode;
private:
    TAbstractCache *mcache;
    TNetworkClient *mclient;
    QTextCodec *mcodec;
    BCodeEditor *meditor;
    quint64 mid;
    SampleModel *mmodel;
    quint64 msenderId;
    TTexProject msource;
    bool mvalid;
    //
    QLineEdit *mledtTitle;
    BInputField *minputTitle;
    QToolButton *mtbtnShowPreview;
    QCheckBox *mcboxEditSource;
    QLineEdit *mledtFileName;
    BInputField *minputFileName;
    QLabel *mlblSize;
    QToolButton *mtbtnSetupFromCurrentDocument;
    QToolButton *mtbtnSetupFromExternalFile;
    TTagWidget *mtgwgt;
    QLabel *mlblSender;
    QLabel *mlblCreationDT;
    QLabel *mlblUpdateDT;
    QSpinBox *msboxRating;
    QComboBox *mcmboxType;
    TListWidget *mlstwgtAuthors;
    QPlainTextEdit *mptedtDescription;
    QPlainTextEdit *mptedtRemark;
public:
    explicit SampleInfoWidget(Mode m, QWidget *parent = 0);
public:
    TAbstractCache *cache() const;
    TNetworkClient *client() const;
    QVariant createRequestData() const;
    BCodeEditor *editor() const;
    bool hasValidInput() const;
    Mode mode() const;
    SampleModel *model() const;
    void restoreState(const QByteArray &state);
    QByteArray saveState() const;
    void setCache(TAbstractCache *cache);
    void setClient(TNetworkClient *client);
    void setEditor(BCodeEditor *editor);
    void setModel(SampleModel *model);
    bool setSample(quint64 sampleId);
    QString title() const;
private:
    TAuthorInfoList authors() const;
    void createAdminRemarkGroup(QHBoxLayout *hlt, bool readOnly = false);
    void createAuthorsGroup(QHBoxLayout *hlt, bool readOnly = false);
    void createDescriptionGroup(QHBoxLayout *hlt, bool readOnly = false);
    void createEditSourceField(QFormLayout *flt);
    void createExtraGroup(QHBoxLayout *hlt, bool readOnly = false);
    void createFileField(QFormLayout *flt, bool readOnly = false);
    void createMainGroup(QVBoxLayout *vlt, bool readOnly = false);
    void createTagsField(QFormLayout *flt, bool readOnly = false);
    void createTitleField(QFormLayout *flt, bool readOnly = false);
    void resetFile(const QString &fileName = QString(), int size = 0);
private slots:
    void checkInputs();
    void setFileName(const QString &fileName);
    void setupFromCurrentDocument();
    void setupFromExternalFile();
    void showPreview();
    void showSenderInfo();
signals:
    void inputValidityChanged(bool valid);
};

#endif // SAMPLEINFOWIDGET_H
