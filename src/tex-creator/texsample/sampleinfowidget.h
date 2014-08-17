/****************************************************************************
**
** Copyright (C) 2012-2014 TeXSample Team
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

class TSampleInfo;
class TTagsWidget;
class TListWidget;

class BCodeEditor;
class BAbstractCodeEditorDocument;
class BInputField;

class QLineEdit;
class QLabel;
class QComboBox;
class QSpinBox;
class QPlainTextEdit;
class QToolButton;
class QByteArray;
class QStringList;
class QTextCodec;

#include <QDialog>
#include <QString>

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
        EditMode,
        UpdateMode,
        ShowMode
    };
public:
    static bool showSelectSampleDialog(QString &fileName, QTextCodec *&codec, QWidget *parent = 0);
public:
    explicit SampleInfoWidget(Mode m, QWidget *parent = 0);
    explicit SampleInfoWidget(Mode m, BCodeEditor *editor, QWidget *parent = 0);
public:
    void setInfo(const TSampleInfo &info);
    void setCheckSourceValidity(bool b);
    void restoreState(const QByteArray &state);
    void restoreSourceState(const QByteArray &state);
    Mode mode() const;
    TSampleInfo info() const;
    bool checkSourceValidity() const;
    QString actualFileName() const;
    QTextCodec *codec() const;
    BAbstractCodeEditorDocument *document() const;
    QByteArray saveState() const;
    QByteArray saveSourceState() const;
    bool isValid() const;
public slots:
    void clear();
    void setFocus();
    void setupFromCurrentDocument();
    void setupFromExternalFile(const QString &fileName = QString(), QTextCodec *codec = 0);
private:
    void init();
    void setProjectSize(int sz = 0);
private slots:
    void documentAvailableChanged(bool available);
    void checkInputs();
    void showSenderInfo();
    void previewSample();
    void setFile(const QString &fn, QTextCodec *codec = 0);
signals:
    void validityChanged(bool valid);
private:
    static QString createFileName(const QString &fn);
private:
    static const QString DateTimeFormat;
private:
    const Mode mmode;
    BCodeEditor *const meditor;
private:
    bool mvalid;
    bool mcheckSource;
    quint64 mid;
    quint64 msenderId;
    QString msenderLogin;
    QString msenderRealName;
    QString mactualFileName;
    QTextCodec *mcodec;
    BAbstractCodeEditorDocument *mdoc;
    QToolButton *mtbtnUseCurrentDocument;
    int mprojectSize;
    QLineEdit *mledtTitle;
    BInputField *minputTitle;
    QLineEdit *mledtFileName;
    BInputField *minputFileName;
    TTagsWidget *mtgswgt;
    QSpinBox *msboxRating;
    QLabel *mlblSize;
    QComboBox *mcmboxType;
    QLabel *mlblSender;
    QLabel *mlblCreationDT;
    QLabel *mlblUpdateDT;
    TListWidget *mlstwgt;
    QPlainTextEdit *mptedtComment;
    QPlainTextEdit *mptedtRemark;
};

#endif // SAMPLEINFOWIDGET_H
