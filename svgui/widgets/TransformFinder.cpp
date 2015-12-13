/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "TransformFinder.h"

#include "base/XmlExportable.h"
#include "transform/TransformFactory.h"
#include "SelectableLabel.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QAction>

TransformFinder::TransformFinder(QWidget *parent) :
    QDialog(parent),
    m_resultsFrame(0),
    m_resultsLayout(0)
{
    setWindowTitle(tr("Find a Transform"));
    
    QGridLayout *mainGrid = new QGridLayout;
    mainGrid->setVerticalSpacing(0);
    setLayout(mainGrid);

    mainGrid->addWidget(new QLabel(tr("Find:")), 0, 0);
    
    QLineEdit *searchField = new QLineEdit;
    mainGrid->addWidget(searchField, 0, 1);
    connect(searchField, SIGNAL(textChanged(const QString &)),
            this, SLOT(searchTextChanged(const QString &)));

//    m_infoLabel = new QLabel(tr("Type in this box to search descriptions of available and known transforms"));
    m_infoLabel = new QLabel;
    mainGrid->addWidget(m_infoLabel, 1, 1);

    m_resultsScroll = new QScrollArea;
    mainGrid->addWidget(m_resultsScroll, 2, 0, 1, 2);
    mainGrid->setRowStretch(2, 10);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel);
    mainGrid->addWidget(bb, 3, 0, 1, 2);
    connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bb, SIGNAL(rejected()), this, SLOT(reject()));

    m_resultsFrame = new QWidget;
    QPalette palette = m_resultsFrame->palette();
    palette.setColor(QPalette::Window, palette.color(QPalette::Base));
    m_resultsFrame->setPalette(palette);
    m_resultsScroll->setPalette(palette);
    m_resultsLayout = new QVBoxLayout;
    m_resultsLayout->setSpacing(0);
    m_resultsLayout->setContentsMargins(0, 0, 0, 0);
    m_resultsFrame->setLayout(m_resultsLayout);
    m_resultsScroll->setWidget(m_resultsFrame);
    m_resultsFrame->show();

    m_noResultsLabel = new QLabel(tr("<br>&nbsp;&nbsp;No results found"));
    m_resultsLayout->addWidget(m_noResultsLabel);
    m_noResultsLabel->hide();

    m_beforeSearchLabel = new QLabel;
    m_resultsLayout->addWidget(m_beforeSearchLabel);
    m_beforeSearchLabel->hide();

    QAction *up = new QAction(tr("Up"), this);
    up->setShortcut(tr("Up"));
    connect(up, SIGNAL(triggered()), this, SLOT(up()));
    addAction(up);

    QAction *down = new QAction(tr("Down"), this);
    down->setShortcut(tr("Down"));
    connect(down, SIGNAL(triggered()), this, SLOT(down()));
    addAction(down);

    QDesktopWidget *desktop = QApplication::desktop();
    QRect available = desktop->availableGeometry();

    int width = available.width() / 2;
    int height = available.height() / 2;
    if (height < 450) {
        if (available.height() > 500) height = 450;
    }
    if (width < 600) {
        if (available.width() > 650) width = 600;
    }

    resize(width, height);
    raise();

    setupBeforeSearchLabel();

    m_upToDateCount = 0;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_timer->start(30);
}

TransformFinder::~TransformFinder()
{
}

void
TransformFinder::setupBeforeSearchLabel()
{
    bool haveInstalled =
        TransformFactory::getInstance()->haveInstalledTransforms();
    bool haveUninstalled =
        TransformFactory::getInstance()->haveUninstalledTransforms();

    m_beforeSearchLabel->setWordWrap(true);
    m_beforeSearchLabel->setOpenExternalLinks(true);
    m_beforeSearchLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    m_beforeSearchLabel->setMargin(12);
    m_beforeSearchLabel->setFixedWidth(this->width() - 40);

    QString base =
        tr("<p>Type some text into the search box to search the descriptions of:<ul><li>All currently installed <a href=\"http://www.vamp-plugins.org/\">Vamp</a> audio feature extraction plugins</li><li>All currently installed <a href=\"http://www.ladspa.org/\">LADSPA</a> audio effects plugins</li><li>Vamp plugins that are not currently installed but that have descriptions published via the semantic web</li></ul>");

    QString nopull =
        tr("<b>Unable to retrieve published descriptions from network!</b>");

    QString noinst =
        tr("<b>No plugins are currently installed!</b>");

    if (haveInstalled) {
        if (haveUninstalled) {
            m_beforeSearchLabel->setText(base);
        } else {
            m_beforeSearchLabel->setText
                (base +
                 tr("<p>%1<br>Perhaps the network connection is down, services are responding too slowly, or a processing problem has occurred.<br>Only the descriptions of installed plugins will be searched.").arg(nopull));
        }
    } else {
        if (haveUninstalled) {
            m_beforeSearchLabel->setText
                (base +
                 tr("<p>%1<br>Only the published descriptions of Vamp feature extraction plugins will be searched.").arg(noinst));
        } else {
            m_beforeSearchLabel->setText
                (base +
                 tr("<p>%1<br>%2<br>Perhaps the network connection is down, or services are responding too slowly.<br>No search results will be available.").arg(noinst).arg(nopull));
        }
    }

    m_beforeSearchLabel->show();
    m_resultsFrame->resize(m_resultsFrame->sizeHint());
}

void
TransformFinder::searchTextChanged(const QString &text)
{
//    cerr << "text is " << text << endl;
    m_newSearchText = text;
}

void
TransformFinder::timeout()
{
    int maxResults = 60;
    
    if (m_newSearchText != "") {

        QString text = m_newSearchText;
        m_newSearchText = "";

        QStringList keywords = text.split(' ', QString::SkipEmptyParts);
        TransformFactory::SearchResults results =
            TransformFactory::getInstance()->search(keywords);
        
//        cerr << results.size() << " result(s)..." << endl;
        
        std::set<TextMatcher::Match> sorted;
        sorted.clear();
        for (TransformFactory::SearchResults::const_iterator j = results.begin();
             j != results.end(); ++j) {
            sorted.insert(j->second);
        }
        
        m_sortedResults.clear();
        for (std::set<TextMatcher::Match>::const_iterator j = sorted.end();
             j != sorted.begin(); ) {
            --j;
            m_sortedResults.push_back(*j);
            if ((int)m_sortedResults.size() == maxResults) break;
        }

        if (m_sortedResults.empty()) m_selectedTransform = "";
        else m_selectedTransform = m_sortedResults.begin()->key;

        m_upToDateCount = 0;

        for (int j = (int)m_labels.size(); j > (int)m_sortedResults.size(); ) {
            m_labels[--j]->hide();
        }

        m_beforeSearchLabel->hide();

        if (m_sortedResults.empty()) {
            m_noResultsLabel->show();
            m_resultsFrame->resize(m_resultsFrame->sizeHint());
        } else {
            m_noResultsLabel->hide();
        }

        if (m_sortedResults.size() < sorted.size()) {
            m_infoLabel->setText
                (tr("Found %n description(s) containing <b>%1</b>, showing the first %2 only",
                    0, int(sorted.size())).arg(text).arg(m_sortedResults.size()));
        } else {
            m_infoLabel->setText
                (tr("Found %n description(s) containing <b>%1</b>",
                    0, int(sorted.size())).arg(text));
        }

        return;
    }

    if (m_upToDateCount >= (int)m_sortedResults.size()) return;

    while (m_upToDateCount < (int)m_sortedResults.size()) {

        int i = m_upToDateCount;

//        cerr << "sorted size = " << m_sortedResults.size() << endl;

        TransformDescription desc;
        TransformId tid = m_sortedResults[i].key;
        TransformFactory *factory = TransformFactory::getInstance();
        TransformFactory::TransformInstallStatus status =
            factory->getTransformInstallStatus(tid);
        QString suffix;

        if (status == TransformFactory::TransformInstalled) {
            desc = factory->getTransformDescription(tid);
        } else {
            desc = factory->getUninstalledTransformDescription(tid);
            suffix = tr("<i> (not installed)</i>");
        }

        QString labelText;
        labelText += tr("%1%2<br><small>")
            .arg(XmlExportable::encodeEntities(desc.name))
            .arg(suffix);

        labelText += "...";
        for (TextMatcher::Match::FragmentMap::const_iterator k =
                 m_sortedResults[i].fragments.begin();
             k != m_sortedResults[i].fragments.end(); ++k) {
            labelText += k->second;
            labelText += "... ";
        }
        labelText += tr("</small>");

        QString selectedText;
        selectedText += tr("<b>%1</b>%2<br>")
            .arg(XmlExportable::encodeEntities
                 (desc.name == "" ? desc.identifier : desc.name))
            .arg(suffix);

        if (desc.longDescription != "") {
            selectedText += tr("<small>%1</small>")
                .arg(XmlExportable::encodeEntities(desc.longDescription));
        } else if (desc.description != "") {
            selectedText += tr("<small>%1</small>")
                .arg(XmlExportable::encodeEntities(desc.description));
        }

        selectedText += tr("<small>");
        if (desc.type != TransformDescription::UnknownType) {
            selectedText += tr("<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&mdash; Plugin type: %1")
                .arg(XmlExportable::encodeEntities(factory->getTransformTypeName(desc.type)));
        }
        if (desc.category != "") {
            selectedText += tr("<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&mdash; Category: %1")
                .arg(XmlExportable::encodeEntities(desc.category));
        }
        selectedText += tr("<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&mdash; System identifier: %1")
            .arg(XmlExportable::encodeEntities(desc.identifier));
        if (desc.infoUrl != "") {
            selectedText += tr("<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&mdash; More information: <a href=\"%1\">%1</a>")
                .arg(desc.infoUrl);
        }
        selectedText += tr("</small>");

        if (i >= (int)m_labels.size()) {
            SelectableLabel *label = new SelectableLabel(m_resultsFrame);
            m_resultsLayout->addWidget(label);
            connect(label, SIGNAL(selectionChanged()), this,
                    SLOT(selectedLabelChanged()));
            connect(label, SIGNAL(doubleClicked()), this,
                    SLOT(labelDoubleClicked()));
            QPalette palette = label->palette();
            label->setPalette(palette);
            m_labels.push_back(label);
        }

        m_labels[i]->setObjectName(desc.identifier);
        m_labels[i]->setFixedWidth(this->width() - 40);
        m_labels[i]->setUnselectedText(labelText);

//        cerr << "selected text: " << selectedText << endl;
        m_labels[i]->setSelectedText(selectedText);

        m_labels[i]->setSelected(m_selectedTransform == desc.identifier);

        if (!m_labels[i]->isVisible()) m_labels[i]->show();

        ++m_upToDateCount;

        if (i == 0) break;
    }

    m_resultsFrame->resize(m_resultsFrame->sizeHint());
}

void
TransformFinder::selectedLabelChanged()
{
    QObject *s = sender();
    m_selectedTransform = "";
    for (int i = 0; i < (int)m_labels.size(); ++i) {
        if (!m_labels[i]->isVisible()) continue;
        if (m_labels[i] == s) {
            if (m_labels[i]->isSelected()) {
                m_selectedTransform = m_labels[i]->objectName();
            }
        } else {
            if (m_labels[i]->isSelected()) {
                m_labels[i]->setSelected(false);
            }
        }
    }
    cerr << "selectedLabelChanged: selected transform is now \""
              << m_selectedTransform << "\"" << endl;
}

void
TransformFinder::labelDoubleClicked()
{
    // The first click should have selected the label already
    if (TransformFactory::getInstance()->getTransformInstallStatus
        (m_selectedTransform) == 
        TransformFactory::TransformInstalled) {
        accept();
    }
}

TransformId
TransformFinder::getTransform() const
{
    return m_selectedTransform;
}

void
TransformFinder::up()
{
    for (int i = 0; i < (int)m_labels.size(); ++i) {
        if (!m_labels[i]->isVisible()) continue;
        if (m_labels[i]->objectName() == m_selectedTransform) {
            if (i > 0) {
                m_labels[i]->setSelected(false);
                m_labels[i-1]->setSelected(true);
                m_selectedTransform = m_labels[i-1]->objectName();
            }
            return;
        }
    }
}

void
TransformFinder::down()
{
    for (int i = 0; i < (int)m_labels.size(); ++i) {
        if (!m_labels[i]->isVisible()) continue;
        if (m_labels[i]->objectName() == m_selectedTransform) {
            if (i+1 < (int)m_labels.size() &&
                m_labels[i+1]->isVisible()) {
                m_labels[i]->setSelected(false);
                m_labels[i+1]->setSelected(true);
                m_selectedTransform = m_labels[i+1]->objectName();
            }
            return;
        }
    }
}

