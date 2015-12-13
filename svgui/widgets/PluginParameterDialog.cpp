/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "PluginParameterDialog.h"

#include "PluginParameterBox.h"
#include "WindowTypeSelector.h"

#include "TextAbbrev.h"
#include "IconLoader.h"

#include <vamp-hostsdk/Plugin.h>
#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginWrapper.h>

#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QMessageBox>
#include <QComboBox>
#include <QCheckBox>
#include <QSettings>
#include <QDialogButtonBox>
#include <QDesktopServices>
#include <QUrl>

#include "base/Debug.h"

PluginParameterDialog::PluginParameterDialog(Vamp::PluginBase *plugin,
					     QWidget *parent) :
    QDialog(parent),
    m_plugin(plugin),
    m_channel(-1),
    m_stepSize(0),
    m_blockSize(0),
    m_windowType(HanningWindow),
    m_parameterBox(0),
    m_currentSelectionOnly(false)
{
    setWindowTitle(tr("Plugin Parameters"));

    QGridLayout *grid = new QGridLayout;
    setLayout(grid);

    QGroupBox *pluginBox = new QGroupBox;
    pluginBox->setTitle(plugin->getType().c_str());
    grid->addWidget(pluginBox, 0, 0);

    QGridLayout *subgrid = new QGridLayout;
    pluginBox->setLayout(subgrid);

    subgrid->setSpacing(0);
    subgrid->setMargin(10);

    QFont boldFont(pluginBox->font());
    boldFont.setBold(true);

    QFont italicFont(pluginBox->font());
    italicFont.setItalic(true);

    QLabel *nameLabel = new QLabel(plugin->getName().c_str());
    nameLabel->setWordWrap(true);
    nameLabel->setFont(boldFont);

    QLabel *makerLabel = new QLabel(plugin->getMaker().c_str());
    makerLabel->setWordWrap(true);

    int version = plugin->getPluginVersion();
    QLabel *versionLabel = new QLabel(QString("%1").arg(version));
    versionLabel->setWordWrap(true);

    QLabel *copyrightLabel = new QLabel(plugin->getCopyright().c_str());
    copyrightLabel->setWordWrap(true);

//    QLabel *typeLabel = new QLabel(plugin->getType().c_str());
//    typeLabel->setWordWrap(true);
//    typeLabel->setFont(boldFont);

    QLabel *descriptionLabel = 0;
    if (plugin->getDescription() != "") {
        descriptionLabel = new QLabel(plugin->getDescription().c_str());
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setFont(italicFont);
    }

    int row = 0;

    QLabel *label = new QLabel(tr("Name:"));
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    subgrid->addWidget(label, row, 0);
    subgrid->addWidget(nameLabel, row, 1);

    m_moreInfo = new QPushButton;
    m_moreInfo->setIcon(IconLoader().load("info"));
    m_moreInfo->setFixedSize(QSize(16, 16));
    connect(m_moreInfo, SIGNAL(clicked()), this, SLOT(moreInfo()));
    subgrid->addWidget(m_moreInfo, row, 2, Qt::AlignTop | Qt::AlignRight);
    m_moreInfo->hide();

    row++;

    if (descriptionLabel) {
//        label = new QLabel(tr("Description:"));
//        label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
//        subgrid->addWidget(label, row, 0);
        subgrid->addWidget(descriptionLabel, row, 1, 1, 2);
        row++;
    }

    if (version >= 0) {
        label = new QLabel(tr("Version:"));
        label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        subgrid->addWidget(label, row, 0);
        subgrid->addWidget(versionLabel, row, 1);
        row++;
    }

//    label = new QLabel(tr("Type:"));
//    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
//    subgrid->addWidget(label, row, 0);
//    subgrid->addWidget(typeLabel, row, 1);
//    row++;

    label = new QLabel(tr("Maker:"));
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    subgrid->addWidget(label, row, 0);
    subgrid->addWidget(makerLabel, row, 1);
    row++;

    label = new QLabel(tr("Copyright:  "));
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    subgrid->addWidget(label, row, 0);
    subgrid->addWidget(copyrightLabel, row, 1);
    row++;
    
    m_outputSpacer = new QLabel;
    subgrid->addWidget(m_outputSpacer, row, 0);
    m_outputSpacer->setFixedHeight(7);
    m_outputSpacer->hide();
    row++;

    m_outputLabel = new QLabel(tr("Output:"));
    m_outputLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    subgrid->addWidget(m_outputLabel, row, 0);
    m_outputValue = new QLabel;
    m_outputValue->setFont(boldFont);
    subgrid->addWidget(m_outputValue, row, 1);
    m_outputLabel->hide();
    m_outputValue->hide();
    row++;

    m_outputDescription = new QLabel;
    m_outputDescription->setFont(italicFont);
    subgrid->addWidget(m_outputDescription, row, 1);
    m_outputDescription->hide();
    row++;

    subgrid->setColumnStretch(1, 2);

    m_inputModelBox = new QGroupBox;
    m_inputModelBox->setTitle(tr("Input Material"));
    grid->addWidget(m_inputModelBox, 1, 0);
    
    m_inputModels = new QComboBox;
    QVBoxLayout *inputLayout = new QVBoxLayout;
    m_inputModelBox->setLayout(inputLayout);
    inputLayout->addWidget(m_inputModels);
    m_inputModels->hide();

    m_selectionOnly = new QCheckBox(tr("Restrict to selection extents"));
    inputLayout->addWidget(m_selectionOnly);
    m_selectionOnly->hide();

    m_inputModelBox->hide();

    QGroupBox *paramBox = new QGroupBox;
    paramBox->setTitle(tr("Plugin Parameters"));
    grid->addWidget(paramBox, 2, 0);
    grid->setRowStretch(2, 10);

    QHBoxLayout *paramLayout = new QHBoxLayout;
    paramLayout->setMargin(0);
    paramBox->setLayout(paramLayout);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);
    paramLayout->addWidget(scroll);

    m_parameterBox = new PluginParameterBox(m_plugin);
    connect(m_parameterBox, SIGNAL(pluginConfigurationChanged(QString)),
            this,  SIGNAL(pluginConfigurationChanged(QString)));
    scroll->setWidget(m_parameterBox);

    m_advanced = new QFrame;
    QVBoxLayout *advancedLayout = new QVBoxLayout;
    advancedLayout->setMargin(0);
    m_advanced->setLayout(advancedLayout);
    grid->addWidget(m_advanced, 3, 0);

    m_channelBox = new QGroupBox;
    m_channelBox->setTitle(tr("Channels"));
    advancedLayout->addWidget(m_channelBox);
    m_channelBox->setVisible(false);
    m_haveChannelBoxData = false;

    m_windowBox = new QGroupBox;
    m_windowBox->setTitle(tr("Processing"));
    advancedLayout->addWidget(m_windowBox);
    m_windowBox->setVisible(false);
    m_haveWindowBoxData = false;

    QHBoxLayout *hbox = new QHBoxLayout;
    grid->addLayout(hbox, 4, 0);

    m_advancedVisible = false;

    m_advancedButton = new QPushButton(tr("Advanced >>"));
    m_advancedButton->setCheckable(true);
    connect(m_advancedButton, SIGNAL(clicked()), this, SLOT(advancedToggled()));
        
    QSettings settings;
    settings.beginGroup("PluginParameterDialog");
    m_advancedVisible = settings.value("advancedvisible", false).toBool();
    settings.endGroup();
    
    m_advanced->hide();

    hbox->addWidget(m_advancedButton);
    m_advancedButton->hide();

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel);
    hbox->addWidget(bb);
    connect(bb, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
    connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
    bb->button(QDialogButtonBox::Ok)->setDefault(true);

    setAdvancedVisible(m_advancedVisible);
}

PluginParameterDialog::~PluginParameterDialog()
{
}

int
PluginParameterDialog::exec()
{
    show();
    setAdvancedVisible(m_advancedVisible);
    return QDialog::exec();
}


void
PluginParameterDialog::setOutputLabel(QString text,
                                      QString description)
{
    if (text == "") {
        m_outputSpacer->hide();
        m_outputLabel->hide();
        m_outputValue->hide();
        m_outputDescription->hide();
    } else {
        m_outputSpacer->show();
        m_outputValue->setText(text);
        m_outputValue->setWordWrap(true);
        m_outputDescription->setText(description);
        m_outputDescription->setWordWrap(true);
        m_outputLabel->show();
        m_outputValue->show();
        if (description != "") {
            m_outputDescription->show();
        } else {
            m_outputDescription->hide();
        }
    }
}

void
PluginParameterDialog::setMoreInfoUrl(QString moreInfoUrl)
{
    m_moreInfoUrl = moreInfoUrl;
    if (m_moreInfoUrl != "") {
        m_moreInfo->show();
    } else {
        m_moreInfo->hide();
    }
}

void
PluginParameterDialog::setChannelArrangement(int sourceChannels,
                                             int targetChannels,
                                             int defaultChannel)
{
    m_channel = defaultChannel;

    if (sourceChannels != targetChannels) {

        // At the moment we can only cope with the case where
        // sourceChannels > targetChannels and targetChannels == 1

        if (sourceChannels < targetChannels) {

            QMessageBox::warning
                (parentWidget(),
                 tr("Channel mismatch"),
                 tr("This plugin requires at least %1 input channels, but only %2 %3 available.  The plugin probably will not work correctly.").arg(targetChannels).arg(sourceChannels).arg(sourceChannels != 1 ? tr("are") : tr("is")),
                 QMessageBox::Ok,
                 QMessageBox::NoButton);

        } else {

            if (m_haveChannelBoxData) {
                cerr << "WARNING: PluginParameterDialog::setChannelArrangement: Calling more than once on same dialog is not currently implemented" << endl;
                return;
            }
            
            QVBoxLayout *channelLayout = new QVBoxLayout;
            m_channelBox->setLayout(channelLayout);

            if (targetChannels != 1) {

                channelLayout->addWidget
                    (new QLabel(tr("This plugin accepts no more than %1 input channels,\nbut %2 are available.  Only the first %3 will be used.\n")
                                .arg(targetChannels)
                                .arg(sourceChannels)
                                .arg(targetChannels)));

            } else {

                channelLayout->addWidget(new QLabel(tr("This plugin only has a single channel input,\nbut the source has %1 channels.").arg(sourceChannels)));

                QComboBox *channelCombo = new QComboBox;
                channelCombo->addItem(tr("Use mean of source channels"));
                for (int i = 0; i < sourceChannels; ++i) {
                    channelCombo->addItem(tr("Use channel %1 only").arg(i + 1));
                }

                connect(channelCombo, SIGNAL(activated(int)),
                        this, SLOT(channelComboChanged(int)));

                channelLayout->addWidget(channelCombo);
            }

            m_channelBox->setVisible(true);
            m_haveChannelBoxData = true;
            m_advancedButton->show();
        }
    }

    setAdvancedVisible(m_advancedVisible);
}

void
PluginParameterDialog::setShowProcessingOptions(bool showWindowSize,
                                                bool showFrequencyDomainOptions)
{
    if (m_haveWindowBoxData) {
        cerr << "WARNING: PluginParameterDialog::setShowProcessingOptions: Calling more than once on same dialog is not currently implemented" << endl;
        return;
    }

    if (showWindowSize) {

        Vamp::Plugin *fePlugin = dynamic_cast<Vamp::Plugin *>(m_plugin);
        if (!fePlugin) fePlugin = dynamic_cast<Vamp::PluginHostAdapter *>(m_plugin);
        if (!fePlugin) fePlugin = dynamic_cast<Vamp::HostExt::PluginWrapper *>(m_plugin);
        int size = 1024;
        int increment = 1024;
        if (fePlugin) {
            size = int(fePlugin->getPreferredBlockSize());
            cerr << "Feature extraction plugin \"" << fePlugin->getName() << "\" reports preferred block size as " << size << endl;
            if (size == 0) size = 1024;
            increment = int(fePlugin->getPreferredStepSize());
            if (increment == 0) {
                if (fePlugin->getInputDomain() == Vamp::Plugin::TimeDomain) {
                    increment = size;
                } else {
                    increment = size/2;
                }
            }
        }

        QGridLayout *windowLayout = new QGridLayout;
        m_windowBox->setLayout(windowLayout);

        if (showFrequencyDomainOptions) {
            windowLayout->addWidget(new QLabel(tr("Window size:")), 0, 0);
        } else {
            windowLayout->addWidget(new QLabel(tr("Audio frames per block:")), 0, 0);
        }

        cerr << "size: " << size << ", increment: " << increment << endl;

        QComboBox *blockSizeCombo = new QComboBox;
        blockSizeCombo->setEditable(true);
        bool found = false;
        for (int i = 0; i < 14; ++i) {
            int val = 1 << (i + 3);
            blockSizeCombo->addItem(QString("%1").arg(val));
            if (val == size) {
                blockSizeCombo->setCurrentIndex(i);
                found = true;
            }
        }
        if (!found) {
            blockSizeCombo->addItem(QString("%1").arg(size));
            blockSizeCombo->setCurrentIndex(blockSizeCombo->count() - 1);
        }
        blockSizeCombo->setValidator(new QIntValidator(1, int(pow(2., 18)), this));
        connect(blockSizeCombo, SIGNAL(editTextChanged(const QString &)),
                this, SLOT(blockSizeComboChanged(const QString &)));
        windowLayout->addWidget(blockSizeCombo, 0, 1);

        windowLayout->addWidget(new QLabel(tr("Window increment:")), 1, 0);
        
        QComboBox *incrementCombo = new QComboBox;
        incrementCombo->setEditable(true);
        found = false;
        for (int i = 0; i < 14; ++i) {
            int val = 1 << (i + 3);
            incrementCombo->addItem(QString("%1").arg(val));
            if (val == increment) {
                incrementCombo->setCurrentIndex(i);
                found = true;
            }
        }
        if (!found) {
            incrementCombo->addItem(QString("%1").arg(increment));
            incrementCombo->setCurrentIndex(incrementCombo->count() - 1);
        }
        incrementCombo->setValidator(new QIntValidator(1, int(pow(2., 18)), this));
        connect(incrementCombo, SIGNAL(editTextChanged(const QString &)),
                this, SLOT(incrementComboChanged(const QString &)));
        windowLayout->addWidget(incrementCombo, 1, 1);
        
        if (showFrequencyDomainOptions) {
            
            windowLayout->addWidget(new QLabel(tr("Window shape:")), 2, 0);
            WindowTypeSelector *windowTypeSelector = new WindowTypeSelector;
            connect(windowTypeSelector, SIGNAL(windowTypeChanged(WindowType)),
                    this, SLOT(windowTypeChanged(WindowType)));
            windowLayout->addWidget(windowTypeSelector, 2, 1);
        }

        m_windowBox->setVisible(true);
        m_haveWindowBoxData = true;
        m_advancedButton->show();
    }

    setAdvancedVisible(m_advancedVisible);
}

void
PluginParameterDialog::setCandidateInputModels(const QStringList &models,
                                               QString defaultModel)
{
    m_inputModels->clear();

    QSettings settings;
    settings.beginGroup("PluginParameterDialog");
    QString lastModel = settings.value("lastinputmodel").toString();
    settings.endGroup();

    if (defaultModel == "") defaultModel = lastModel;

    m_inputModels->show();

    m_inputModelList = models;
    m_inputModels->addItems(TextAbbrev::abbreviate(models, 80));
    m_inputModels->setCurrentIndex(0);

    if (defaultModel != "") {
        for (int i = 0; i < models.size(); ++i) {
            if (defaultModel == models[i]) {
                m_inputModels->setCurrentIndex(i);
                m_currentInputModel = models[i];
                break;
            }
        }
    }

    connect(m_inputModels, SIGNAL(activated(int)),
            this, SLOT(inputModelComboChanged(int)));
    m_inputModelBox->show();
}

void
PluginParameterDialog::setShowSelectionOnlyOption(bool show)
{
    if (!show) {
        m_selectionOnly->hide();
        if (!m_inputModels->isVisible()) m_inputModelBox->hide();
        return;
    }

    QSettings settings;
    settings.beginGroup("PluginParameterDialog");
    bool lastSelectionOnly = settings.value("lastselectiononly", false).toBool();
    settings.endGroup();

    m_selectionOnly->setChecked(lastSelectionOnly);
    m_currentSelectionOnly = lastSelectionOnly;

    connect(m_selectionOnly, SIGNAL(stateChanged(int)),
            this, SLOT(selectionOnlyChanged(int)));

    m_selectionOnly->show();
    m_inputModelBox->show();
}

QString
PluginParameterDialog::getInputModel() const
{
    return m_currentInputModel;
}

bool
PluginParameterDialog::getSelectionOnly() const
{
    return m_currentSelectionOnly;
}

void
PluginParameterDialog::getProcessingParameters(int &blockSize) const
{
    blockSize = m_blockSize;
    return;
}

void
PluginParameterDialog::getProcessingParameters(int &stepSize,
                                               int &blockSize,
                                               WindowType &windowType) const
{
    stepSize = m_stepSize;
    blockSize = m_blockSize;
    windowType = m_windowType;
    return;
}

void
PluginParameterDialog::blockSizeComboChanged(const QString &text)
{
    m_blockSize = text.toInt();
    cerr << "Block size changed to " << m_blockSize << endl;
}

void
PluginParameterDialog::incrementComboChanged(const QString &text)
{
    m_stepSize = text.toInt();
    //!!! rename increment to step size throughout
    cerr << "Increment changed to " << m_stepSize << endl;
}

void
PluginParameterDialog::windowTypeChanged(WindowType type)
{
    m_windowType = type;
}

void
PluginParameterDialog::moreInfo()
{
    if (m_moreInfoUrl != "") {
        QDesktopServices::openUrl(QUrl(m_moreInfoUrl));
    }
}

void
PluginParameterDialog::advancedToggled()
{
    setAdvancedVisible(!m_advancedVisible);
}

void
PluginParameterDialog::setAdvancedVisible(bool visible)
{
//    m_advanced->setVisible(visible);

    if (visible) {
        m_advancedButton->setText(tr("Advanced <<"));
        m_advancedButton->setChecked(true);
        m_advanced->show();
    } else {
        m_advanced->hide();
        m_advancedButton->setText(tr("Advanced >>"));
        m_advancedButton->setChecked(false);
    }

//    cerr << "resize to " << sizeHint().width() << " x " << sizeHint().height() << endl;

//    setMinimumHeight(sizeHint().height());
    adjustSize();

//    (sizeHint());

    m_advancedVisible = visible;

    QSettings settings;
    settings.beginGroup("PluginParameterDialog");
    settings.setValue("advancedvisible", visible);
    settings.endGroup();

//    if (visible) setMaximumHeight(sizeHint().height());
//    adjustSize();
}

void
PluginParameterDialog::channelComboChanged(int index)
{
    m_channel = index - 1;
}

void
PluginParameterDialog::inputModelComboChanged(int index)
{
    if (index >= m_inputModelList.size()) return;
    m_currentInputModel = m_inputModelList[index];
    emit inputModelChanged(m_currentInputModel);
}

void
PluginParameterDialog::selectionOnlyChanged(int state)
{
    if (state == Qt::Checked) {
        m_currentSelectionOnly = true;
    } else {
        m_currentSelectionOnly = false;
    }
}

void
PluginParameterDialog::dialogAccepted()
{
    QSettings settings;
    settings.beginGroup("PluginParameterDialog");

    if (m_inputModels->isVisible()) {
        settings.setValue("lastinputmodel", getInputModel());
    }

    if (m_selectionOnly->isVisible()) {
        settings.setValue("lastselectiononly", getSelectionOnly());
    }

    settings.endGroup();
    
    accept();
}

