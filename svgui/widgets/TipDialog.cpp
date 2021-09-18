/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "TipDialog.h"

#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QLabel>
#include <QLocale>
#include <QXmlInputSource>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>

#include <iostream>

TipDialog::TipDialog(QWidget *parent) :
    QDialog(parent),
    m_tipNumber(0),
    m_label(0),
    m_caption(tr("Tip of the Day"))
{
    readTips();

    QSettings settings;
    settings.beginGroup("TipOfTheDay");

    if (!settings.value("showonstartup", true).toBool()) return;

    m_tipNumber = settings.value("nexttip", 0).toInt();

    setWindowTitle(m_caption);

    QGridLayout *grid = new QGridLayout;
    setLayout(grid);
    
    QGroupBox *groupBox = new QGroupBox;
//    groupBox->setTitle(m_caption);
    grid->addWidget(groupBox, 0, 0);
    
    QGridLayout *subgrid = new QGridLayout;
    groupBox->setLayout(subgrid);

    m_label = new QLabel;
    subgrid->addWidget(m_label, 0, 0);
    m_label->setWordWrap(true);

    QHBoxLayout *hbox = new QHBoxLayout;
    grid->addLayout(hbox, 1, 0);

    QCheckBox *show = new QCheckBox(tr("Show tip on startup"));
    hbox->addWidget(show);

    hbox->addSpacing(20);
    hbox->addStretch(10);
    
    QPushButton *prev = new QPushButton(tr("<< Previous"));
    hbox->addWidget(prev);
    connect(prev, &QAbstractButton::clicked, this, &TipDialog::previous);
    
    QPushButton *next = new QPushButton(tr("Next >>"));
    hbox->addWidget(next);
    connect(next, &QAbstractButton::clicked, this, &TipDialog::next);
    
    QPushButton *close = new QPushButton(tr("Close"));
    hbox->addWidget(close);
    connect(close, &QAbstractButton::clicked, this, &QDialog::accept);
    
    close->setDefault(true);

    showTip();
}

TipDialog::~TipDialog()
{
}

void
TipDialog::next()
{
    if (++m_tipNumber >= int(m_tips.size())) {
        //!!! The tips file should define where we loop back to -- the
        // first one at least is likely to be a generic welcome message
        m_tipNumber = 0;
    }

    showTip();
}

void
TipDialog::previous()
{
    if (--m_tipNumber < 0) {
        m_tipNumber = int(m_tips.size()) - 1;
    }

    showTip();
}

void
TipDialog::readTips()
{
    cerr << "TipDialog::readTips" << endl;

    QString language = QLocale::system().name();
    QString filename = QString(":i18n/tips_%1.xml").arg(language);

    if (!QFileInfo(filename).exists()) {

        QString base = language.section('_', 0, 0);
        filename = QString(":i18n/tips_%1.xml").arg(base);

        if (!QFileInfo(filename).exists()) {

            filename = QString(":i18n/tips.xml");

            if (!QFileInfo(filename).exists()) return;
        }
    }

    QFile file(filename);

    cerr << "TipDialog::readTips from " << filename << endl;

    QXmlInputSource source(&file);
    
    TipFileParser parser(this);
    parser.parse(source);
}

void
TipDialog::showTip()
{
    if (m_tipNumber < int(m_tips.size())) {
        cerr << "Tip " << m_tipNumber << " is: " << m_tips[m_tipNumber] << endl;
        m_label->setText(m_tips[m_tipNumber]);
    } else {
        accept();
    }

    int tn = m_tipNumber;
    if (++tn >= int(m_tips.size())) tn = 0; //!!! as above

    QSettings settings;
    settings.beginGroup("TipOfTheDay");
    settings.setValue("nexttip", tn);
}

TipDialog::TipFileParser::TipFileParser(TipDialog *dialog) :
    m_dialog(dialog),
    m_inTip(false),
    m_inText(false),
    m_inHtml(false)
{
}

TipDialog::TipFileParser::~TipFileParser()
{
}

void
TipDialog::TipFileParser::parse(QXmlInputSource &source)
{
    QXmlSimpleReader reader;
    reader.setContentHandler(this);
    reader.setErrorHandler(this);
    reader.parse(source);
}

bool
TipDialog::TipFileParser::startElement(const QString &, const QString &,
                                       const QString &qName,
                                       const QXmlAttributes &attributes)
{
    QString name = qName.toLower();

    cerr << "TipFileParser::startElement(" << name << ")" << endl;

    if (name == "tips") {
        QString caption = attributes.value("caption");
        cerr << "TipFileParser::caption = " << caption << endl;
        if (caption != "") m_dialog->m_caption = caption;
    } else if (name == "tip") {
        if (m_inTip) {
            cerr << "WARNING: TipFileParser: nested <tip> elements" << endl;
        }
        m_inTip = true;
    } else if (name == "text") {
        if (m_inTip) {
            m_inText = true;
            cerr << "TipFileParser: adding new tip" << endl;
            m_dialog->m_tips.push_back("");
        } else {
            cerr << "WARNING: TipFileParser: <text> outside <tip> element" << endl;
        }
    } else if (name == "html") {
        if (m_inTip) {
            m_inHtml = true;
            cerr << "TipFileParser: adding new tip" << endl;
            m_dialog->m_tips.push_back("");
        } else {
            cerr << "WARNING: TipFileParser: <html> outside <tip> element" << endl;
        }
    } else if (m_inHtml) {
        m_dialog->m_tips[m_dialog->m_tips.size()-1] += "<" + qName;
        for (int i = 0; i < attributes.count(); ++i) {
            m_dialog->m_tips[m_dialog->m_tips.size()-1] += 
                " " + attributes.qName(i) + "=\"" + attributes.value(i) + "\"";
        }
        m_dialog->m_tips[m_dialog->m_tips.size()-1] += ">";
    }
    
    cerr << "TipFileParser::startElement done" << endl;
    return true;
}

bool
TipDialog::TipFileParser::endElement(const QString &, const QString &,
                                     const QString &qName)
{
    QString name = qName.toLower();

    if (name == "text") {
        if (!m_inText) {
            cerr << "WARNING: TipFileParser: </text> without <text>" << endl;
        }
        m_inText = false;
    } else if (name == "html") {
        if (!m_inHtml) {
            cerr << "WARNING: TipFileParser: </html> without <html>" << endl;
        }
        m_inHtml = false;
    } else if (name == "tip") {
        if (m_inText) {
            cerr << "WARNING: TipFileParser: <text> without </text>" << endl;
        } else if (m_inHtml) {
            cerr << "WARNING: TipFileParser: <html> without </html>" << endl;
        } else if (!m_inTip) {
            cerr << "WARNING: TipFileParser: </tip> without <tip>" << endl;
        }
        m_inTip = false;
    } else if (m_inHtml) {
        m_dialog->m_tips[m_dialog->m_tips.size()-1] += "</" + qName + ">";
    }

    return true;
}

bool
TipDialog::TipFileParser::characters(const QString &text)
{
    cerr << "TipFileParser::characters(" << text << ")" << endl;

    if (m_inText || m_inHtml) {
        m_dialog->m_tips[m_dialog->m_tips.size()-1] += text;
    }

    return true;
}

bool
TipDialog::TipFileParser::error(const QXmlParseException &exception)
{
    QString errorString =
	QString("ERROR: TipFileParser: %1 at line %2, column %3")
	.arg(exception.message())
	.arg(exception.lineNumber())
	.arg(exception.columnNumber());
    cerr << errorString << endl;
    return QXmlDefaultHandler::error(exception);
}

bool
TipDialog::TipFileParser::fatalError(const QXmlParseException &exception)
{
    QString errorString =
	QString("FATAL ERROR: TipFileParser: %1 at line %2, column %3")
	.arg(exception.message())
	.arg(exception.lineNumber())
	.arg(exception.columnNumber());
    cerr << errorString << endl;
    return QXmlDefaultHandler::fatalError(exception);
}
