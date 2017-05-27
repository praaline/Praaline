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

#include "PropertyBox.h"
#include "PluginParameterDialog.h"

#include "base/PropertyContainer.h"
#include "base/PlayParameters.h"
#include "base/PlayParameterRepository.h"
#include "layer/Layer.h"
#include "layer/ColourDatabase.h"
#include "base/UnitDatabase.h"
#include "base/RangeMapper.h"

#include "AudioDial.h"
#include "LEDButton.h"
#include "IconLoader.h"

#include "NotifyingCheckBox.h"
#include "NotifyingComboBox.h"
#include "NotifyingPushButton.h"
#include "ColourNameDialog.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QApplication>
#include <QColorDialog>
#include <QInputDialog>
#include <QDir>

#include <cassert>
#include <iostream>
#include <cmath>

//#define DEBUG_PROPERTY_BOX 1

PropertyBox::PropertyBox(PropertyContainer *container) :
    m_container(container),
    m_showButton(0),
    m_playButton(0)
{
#ifdef DEBUG_PROPERTY_BOX
    cerr << "PropertyBox[" << this << "(\"" <<
	container->getPropertyContainerName() << "\" at " << container << ")]::PropertyBox" << endl;
#endif

    m_mainBox = new QVBoxLayout;
    setLayout(m_mainBox);

//    m_nameWidget = new QLabel;
//    m_mainBox->addWidget(m_nameWidget);
//    m_nameWidget->setText(container->objectName());

    m_mainWidget = new QWidget;
    m_mainBox->addWidget(m_mainWidget);
    m_mainBox->insertStretch(2, 10);

    m_viewPlayFrame = 0;
    populateViewPlayFrame();

    m_layout = new QGridLayout;
    m_layout->setMargin(0);
    m_layout->setHorizontalSpacing(2);
    m_layout->setVerticalSpacing(1);
    m_mainWidget->setLayout(m_layout);

    PropertyContainer::PropertyList properties = m_container->getProperties();

    blockSignals(true);

    size_t i;

    for (i = 0; i < properties.size(); ++i) {
	updatePropertyEditor(properties[i]);
    }

    blockSignals(false);

    m_layout->setRowStretch(m_layout->rowCount(), 10);

    connect(UnitDatabase::getInstance(), SIGNAL(unitDatabaseChanged()),
            this, SLOT(unitDatabaseChanged()));

    connect(ColourDatabase::getInstance(), SIGNAL(colourDatabaseChanged()),
            this, SLOT(colourDatabaseChanged()));

#ifdef DEBUG_PROPERTY_BOX
    cerr << "PropertyBox[" << this << "]::PropertyBox returning" << endl;
#endif
}

PropertyBox::~PropertyBox()
{
#ifdef DEBUG_PROPERTY_BOX
    cerr << "PropertyBox[" << this << "]::~PropertyBox" << endl;
#endif
}

void
PropertyBox::populateViewPlayFrame()
{
#ifdef DEBUG_PROPERTY_BOX
    cerr << "PropertyBox[" << this << ":" << m_container << "]::populateViewPlayFrame" << endl;
#endif

    if (m_viewPlayFrame) {
	delete m_viewPlayFrame;
	m_viewPlayFrame = 0;
    }

    if (!m_container) return;

    Layer *layer = dynamic_cast<Layer *>(m_container);
    if (layer) {
	disconnect(layer, SIGNAL(modelReplaced()),
                   this, SLOT(populateViewPlayFrame()));
	connect(layer, SIGNAL(modelReplaced()),
		this, SLOT(populateViewPlayFrame()));
    }

    PlayParameters *params = m_container->getPlayParameters();
    if (!params && !layer) return;

    m_viewPlayFrame = new QFrame;
    m_viewPlayFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_mainBox->addWidget(m_viewPlayFrame);

    QHBoxLayout *layout = new QHBoxLayout;
    m_viewPlayFrame->setLayout(layout);

    layout->setMargin(layout->margin() / 2);

#ifdef DEBUG_PROPERTY_BOX
    cerr << "PropertyBox::populateViewPlayFrame: container " << m_container << " (name " << m_container->getPropertyContainerName() << ") params " << params << endl;
#endif

    if (layer) {
	QLabel *showLabel = new QLabel(tr("Show"));
	layout->addWidget(showLabel);
	layout->setAlignment(showLabel, Qt::AlignVCenter);

	m_showButton = new LEDButton(Qt::blue);
	layout->addWidget(m_showButton);
	connect(m_showButton, SIGNAL(stateChanged(bool)),
		this, SIGNAL(showLayer(bool)));
        connect(m_showButton, SIGNAL(mouseEntered()),
                this, SLOT(mouseEnteredWidget()));
        connect(m_showButton, SIGNAL(mouseLeft()),
                this, SLOT(mouseLeftWidget()));
	layout->setAlignment(m_showButton, Qt::AlignVCenter);
    }
    
    if (params) {

	QLabel *playLabel = new QLabel(tr("Play"));
	layout->addWidget(playLabel);
	layout->setAlignment(playLabel, Qt::AlignVCenter);

	m_playButton = new LEDButton(Qt::darkGreen);
        m_playButton->setState(!params->isPlayMuted());
	layout->addWidget(m_playButton);
	connect(m_playButton, SIGNAL(stateChanged(bool)),
		this, SLOT(playAudibleButtonChanged(bool)));
        connect(m_playButton, SIGNAL(mouseEntered()),
                this, SLOT(mouseEnteredWidget()));
        connect(m_playButton, SIGNAL(mouseLeft()),
                this, SLOT(mouseLeftWidget()));
	connect(params, SIGNAL(playAudibleChanged(bool)),
		this, SLOT(playAudibleChanged(bool)));
	layout->setAlignment(m_playButton, Qt::AlignVCenter);

	layout->insertStretch(-1, 10);

        if (params->getPlayClipId() != "") {
            QPushButton *playParamButton =
                new QPushButton(QIcon(":icons/faders.png"), "");
            playParamButton->setFixedWidth(24);
            playParamButton->setFixedHeight(24);
            layout->addWidget(playParamButton);
            connect(playParamButton, SIGNAL(clicked()),
                    this, SLOT(editPlayParameters()));
        }

	AudioDial *gainDial = new AudioDial;
	layout->addWidget(gainDial);
	gainDial->setMeterColor(Qt::darkRed);
	gainDial->setMinimum(-50);
	gainDial->setMaximum(50);
	gainDial->setPageStep(1);
	gainDial->setFixedWidth(24);
	gainDial->setFixedHeight(24);
	gainDial->setNotchesVisible(false);
	gainDial->setDefaultValue(0);
        gainDial->setObjectName(tr("Playback Gain"));
        gainDial->setRangeMapper(new LinearRangeMapper
                                 (-50, 50, -25, 25, tr("dB")));
        gainDial->setShowToolTip(true);
	connect(gainDial, SIGNAL(valueChanged(int)),
		this, SLOT(playGainDialChanged(int)));
	connect(params, SIGNAL(playGainChanged(float)),
		this, SLOT(playGainChanged(float)));
	connect(this, SIGNAL(changePlayGainDial(int)),
		gainDial, SLOT(setValue(int)));
        connect(gainDial, SIGNAL(mouseEntered()),
                this, SLOT(mouseEnteredWidget()));
        connect(gainDial, SIGNAL(mouseLeft()),
                this, SLOT(mouseLeftWidget()));
        playGainChanged(params->getPlayGain());
	layout->setAlignment(gainDial, Qt::AlignVCenter);

	AudioDial *panDial = new AudioDial;
	layout->addWidget(panDial);
	panDial->setMeterColor(Qt::darkGreen);
	panDial->setMinimum(-50);
	panDial->setMaximum(50);
	panDial->setPageStep(1);
	panDial->setFixedWidth(24);
	panDial->setFixedHeight(24);
	panDial->setNotchesVisible(false);
	panDial->setToolTip(tr("Playback Pan / Balance"));
	panDial->setDefaultValue(0);
        panDial->setObjectName(tr("Playback Pan / Balance"));
        panDial->setShowToolTip(true);
	connect(panDial, SIGNAL(valueChanged(int)),
		this, SLOT(playPanDialChanged(int)));
	connect(params, SIGNAL(playPanChanged(float)),
		this, SLOT(playPanChanged(float)));
	connect(this, SIGNAL(changePlayPanDial(int)),
		panDial, SLOT(setValue(int)));
        connect(panDial, SIGNAL(mouseEntered()),
                this, SLOT(mouseEnteredWidget()));
        connect(panDial, SIGNAL(mouseLeft()),
                this, SLOT(mouseLeftWidget()));
        playPanChanged(params->getPlayPan());
	layout->setAlignment(panDial, Qt::AlignVCenter);

    } else {

	layout->insertStretch(-1, 10);
    }
}

void
PropertyBox::updatePropertyEditor(PropertyContainer::PropertyName name,
                                  bool rangeChanged)
{
    PropertyContainer::PropertyType type = m_container->getPropertyType(name);
    int row = m_layout->rowCount();

    int min = 0, max = 0, value = 0, deflt = 0;
    value = m_container->getPropertyRangeAndValue(name, &min, &max, &deflt);

    bool have = (m_propertyControllers.find(name) !=
		 m_propertyControllers.end());

    QString groupName = m_container->getPropertyGroupName(name);
    QString propertyLabel = m_container->getPropertyLabel(name);
    QString iconName = m_container->getPropertyIconName(name);

#ifdef DEBUG_PROPERTY_BOX
    cerr << "PropertyBox[" << this
	      << "(\"" << m_container->getPropertyContainerName()
	      << "\")]";
    cerr << "::updatePropertyEditor(\"" << name << "\"):";
    cerr << " value " << value << ", have " << have << ", group \""
	      << groupName << "\"" << endl;
#endif

    bool inGroup = (groupName != QString());

    if (!have) {
	if (inGroup) {
	    if (m_groupLayouts.find(groupName) == m_groupLayouts.end()) {
#ifdef DEBUG_PROPERTY_BOX
		cerr << "PropertyBox: adding label \"" << groupName << "\" and frame for group for \"" << name << "\"" << endl;
#endif
		m_layout->addWidget(new QLabel(groupName, m_mainWidget), row, 0);
		QFrame *frame = new QFrame(m_mainWidget);
		m_layout->addWidget(frame, row, 1, 1, 2);
		m_groupLayouts[groupName] = new QGridLayout;
		m_groupLayouts[groupName]->setMargin(0);
		frame->setLayout(m_groupLayouts[groupName]);
	    }
	} else {
#ifdef DEBUG_PROPERTY_BOX 
	    cerr << "PropertyBox: adding label \"" << propertyLabel << "\"" << endl;
#endif
	    m_layout->addWidget(new QLabel(propertyLabel, m_mainWidget), row, 0);
	}
    }

    switch (type) {

    case PropertyContainer::ToggleProperty:
    {
        QAbstractButton *button = 0;

	if (have) {
            button = dynamic_cast<QAbstractButton *>(m_propertyControllers[name]);
            assert(button);
	} else {
#ifdef DEBUG_PROPERTY_BOX 
	    cerr << "PropertyBox: creating new checkbox" << endl;
#endif
            if (iconName != "") {
                button = new NotifyingPushButton();
                button->setCheckable(true);
                QIcon icon(IconLoader().load(iconName));
                button->setIcon(icon);
                button->setObjectName(name);
                button->setFixedSize(QSize(18, 18));
            } else {
                button = new NotifyingCheckBox();
                button->setObjectName(name);
            }
	    connect(button, SIGNAL(toggled(bool)),
		    this, SLOT(propertyControllerChanged(bool)));
            connect(button, SIGNAL(mouseEntered()),
                    this, SLOT(mouseEnteredWidget()));
            connect(button, SIGNAL(mouseLeft()),
                    this, SLOT(mouseLeftWidget()));
	    if (inGroup) {
		button->setToolTip(propertyLabel);
		m_groupLayouts[groupName]->addWidget
                    (button, 0, m_groupLayouts[groupName]->columnCount());
	    } else {
		m_layout->addWidget(button, row, 1, 1, 2);
	    }
	    m_propertyControllers[name] = button;
	}

        if (button->isChecked() != (value > 0)) {
	    button->blockSignals(true);
	    button->setChecked(value > 0);
	    button->blockSignals(false);
	}
	break;
    }

    case PropertyContainer::RangeProperty:
    {
	AudioDial *dial;

	if (have) {
	    dial = dynamic_cast<AudioDial *>(m_propertyControllers[name]);
	    assert(dial);
            if (rangeChanged) {
                dial->blockSignals(true);
                dial->setMinimum(min);
                dial->setMaximum(max);
                dial->setRangeMapper(m_container->getNewPropertyRangeMapper(name));
                dial->blockSignals(false);
            }
                
	} else {
#ifdef DEBUG_PROPERTY_BOX 
	    cerr << "PropertyBox: creating new dial" << endl;
#endif
	    dial = new AudioDial();
	    dial->setObjectName(name);
	    dial->setMinimum(min);
	    dial->setMaximum(max);
	    dial->setPageStep(1);
	    dial->setNotchesVisible((max - min) <= 12);
	    dial->setDefaultValue(deflt);
            dial->setRangeMapper(m_container->getNewPropertyRangeMapper(name));
            dial->setShowToolTip(true);
	    connect(dial, SIGNAL(valueChanged(int)),
		    this, SLOT(propertyControllerChanged(int)));
            connect(dial, SIGNAL(mouseEntered()),
                    this, SLOT(mouseEnteredWidget()));
            connect(dial, SIGNAL(mouseLeft()),
                    this, SLOT(mouseLeftWidget()));

	    if (inGroup) {
		dial->setFixedWidth(24);
		dial->setFixedHeight(24);
		m_groupLayouts[groupName]->addWidget
                    (dial, 0, m_groupLayouts[groupName]->columnCount());
	    } else {
		dial->setFixedWidth(32);
		dial->setFixedHeight(32);
		m_layout->addWidget(dial, row, 1);
		QLabel *label = new QLabel(m_mainWidget);
		connect(dial, SIGNAL(valueChanged(int)),
			label, SLOT(setNum(int)));
		label->setNum(value);
		m_layout->addWidget(label, row, 2);
	    }

	    m_propertyControllers[name] = dial;
	}

	if (dial->value() != value) {
	    dial->blockSignals(true);
	    dial->setValue(value);
	    dial->blockSignals(false);
	}
	break;
    }

    case PropertyContainer::ValueProperty:
    case PropertyContainer::UnitsProperty:
    case PropertyContainer::ColourProperty:
    {
	NotifyingComboBox *cb;

	if (have) {
	    cb = dynamic_cast<NotifyingComboBox *>(m_propertyControllers[name]);
	    assert(cb);
	} else {
#ifdef DEBUG_PROPERTY_BOX 
	    cerr << "PropertyBox: creating new combobox" << endl;
#endif

	    cb = new NotifyingComboBox();
	    cb->setObjectName(name);
            cb->setDuplicatesEnabled(false);
        }

        if (!have || rangeChanged) {

            cb->blockSignals(true);
            cb->clear();
            cb->setEditable(false);

            if (type == PropertyContainer::ValueProperty) {

                for (int i = min; i <= max; ++i) {
                    cb->addItem(m_container->getPropertyValueLabel(name, i));
                }

            } else if (type == PropertyContainer::UnitsProperty) {

                QStringList units = UnitDatabase::getInstance()->getKnownUnits();
                for (int i = 0; i < units.size(); ++i) {
                    cb->addItem(units[i]);
                }

                cb->setEditable(true);

            } else { // ColourProperty

                //!!! should be a proper colour combobox class that
                // manages its own Add New Colour entry...
                
                ColourDatabase *db = ColourDatabase::getInstance();
                for (int i = 0; i < db->getColourCount(); ++i) {
                    QString name = db->getColourName(i);
                    cb->addItem(db->getExamplePixmap(i, QSize(12, 12)), name);
                }
                cb->addItem(tr("Add New Colour..."));
            }                
                
            cb->blockSignals(false);
            if (cb->count() < 20 && cb->count() > cb->maxVisibleItems()) {
                cb->setMaxVisibleItems(cb->count());
            }
        }

        if (!have) {
	    connect(cb, SIGNAL(activated(int)),
		    this, SLOT(propertyControllerChanged(int)));
            connect(cb, SIGNAL(mouseEntered()),
                    this, SLOT(mouseEnteredWidget()));
            connect(cb, SIGNAL(mouseLeft()),
                    this, SLOT(mouseLeftWidget()));

	    if (inGroup) {
		cb->setToolTip(propertyLabel);
		m_groupLayouts[groupName]->addWidget
                    (cb, 0, m_groupLayouts[groupName]->columnCount());
	    } else {
		m_layout->addWidget(cb, row, 1, 1, 2);
	    }
	    m_propertyControllers[name] = cb;
	}

        cb->blockSignals(true);
        if (type == PropertyContainer::ValueProperty ||
            type == PropertyContainer::ColourProperty) {
            if (cb->currentIndex() != value) {
                cb->setCurrentIndex(value);
            }
        } else {
            QString unit = UnitDatabase::getInstance()->getUnitById(value);
            if (cb->currentText() != unit) {
                for (int i = 0; i < cb->count(); ++i) {
                    if (cb->itemText(i) == unit) {
                        cb->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }
        cb->blockSignals(false);

#ifdef Q_OS_MAC
	// Crashes on startup without this, for some reason
	cb->setMinimumSize(QSize(10, 10));
#endif

	break;
    }

    case PropertyContainer::InvalidProperty:
    default:
	break;
    }
}

void
PropertyBox::propertyContainerPropertyChanged(PropertyContainer *pc)
{
    if (pc != m_container) return;
    
#ifdef DEBUG_PROPERTY_BOX
    cerr << "PropertyBox::propertyContainerPropertyChanged" << endl;
#endif

    PropertyContainer::PropertyList properties = m_container->getProperties();
    size_t i;

    blockSignals(true);

    for (i = 0; i < properties.size(); ++i) {
	updatePropertyEditor(properties[i]);
    }

    blockSignals(false);
}

void
PropertyBox::propertyContainerPropertyRangeChanged(PropertyContainer *)
{
    blockSignals(true);

    PropertyContainer::PropertyList properties = m_container->getProperties();
    for (size_t i = 0; i < properties.size(); ++i) {
	updatePropertyEditor(properties[i], true);
    }

    blockSignals(false);
}    

void
PropertyBox::unitDatabaseChanged()
{
#ifdef DEBUG_PROPERTY_BOX
    cerr << "PropertyBox[" << this << "]: unitDatabaseChanged" << endl;
#endif
    blockSignals(true);

//    cerr << "my container is " << m_container << endl;
//    cerr << "my container's name is... " << endl;
//    cerr << m_container->objectName() << endl;

    PropertyContainer::PropertyList properties = m_container->getProperties();
    for (size_t i = 0; i < properties.size(); ++i) {
        if (m_container->getPropertyType(properties[i]) ==
            PropertyContainer::UnitsProperty) {
            updatePropertyEditor(properties[i]);
        }
    }

    blockSignals(false);
}    

void
PropertyBox::colourDatabaseChanged()
{
    blockSignals(true);

    PropertyContainer::PropertyList properties = m_container->getProperties();
    for (size_t i = 0; i < properties.size(); ++i) {
        if (m_container->getPropertyType(properties[i]) ==
            PropertyContainer::ColourProperty) {
            updatePropertyEditor(properties[i], true);
        }
    }

    blockSignals(false);
}    

void
PropertyBox::propertyControllerChanged(bool on)
{
    propertyControllerChanged(on ? 1 : 0);
}

void
PropertyBox::propertyControllerChanged(int value)
{
    QObject *obj = sender();
    QString name = obj->objectName();

#ifdef DEBUG_PROPERTY_BOX
    cerr << "PropertyBox::propertyControllerChanged(" << name	      << ", " << value << ")" << endl;
#endif
    
    PropertyContainer::PropertyType type = m_container->getPropertyType(name);

    UndoableCommand *c = 0;

    if (type == PropertyContainer::UnitsProperty) {

        NotifyingComboBox *cb = dynamic_cast<NotifyingComboBox *>(obj);
        if (cb) {
            QString unit = cb->currentText();
            c = m_container->getSetPropertyCommand
                (name, UnitDatabase::getInstance()->getUnitId(unit));
        }

    } else if (type == PropertyContainer::ColourProperty) {

        if (value == int(ColourDatabase::getInstance()->getColourCount())) {
            addNewColour();
            if (value == int(ColourDatabase::getInstance()->getColourCount())) {
                propertyContainerPropertyChanged(m_container);
                return;
            }
        }
        c = m_container->getSetPropertyCommand(name, value);

    } else if (type != PropertyContainer::InvalidProperty) {

	c = m_container->getSetPropertyCommand(name, value);
    }

    if (c) CommandHistory::getInstance()->addCommand(c, true, true);
    
    updateContextHelp(obj);
}

void
PropertyBox::addNewColour()
{
    QColor newColour = QColorDialog::getColor();
    if (!newColour.isValid()) return;

    ColourNameDialog dialog(tr("Name New Colour"),
                            tr("Enter a name for the new colour:"),
                            newColour, newColour.name(), this);
    dialog.showDarkBackgroundCheckbox(tr("Prefer black background for this colour"));
    if (dialog.exec() == QDialog::Accepted) {
        //!!! command
        ColourDatabase *db = ColourDatabase::getInstance();
        int index = db->addColour(newColour, dialog.getColourName());
        db->setUseDarkBackground(index, dialog.isDarkBackgroundChecked());
    }
}

void
PropertyBox::playAudibleChanged(bool audible)
{
    m_playButton->setState(audible);
}

void
PropertyBox::playAudibleButtonChanged(bool audible)
{
    PlayParameters *params = m_container->getPlayParameters();
    if (!params) return;

    if (params->isPlayAudible() != audible) {
        PlayParameterRepository::EditCommand *command =
            new PlayParameterRepository::EditCommand(params);
        command->setPlayAudible(audible);
        CommandHistory::getInstance()->addCommand(command, true, true);
    }
}
    
void
PropertyBox::playGainChanged(float gain)
{
    int dialValue = int(lrint(log10(gain) * 20.0));
    if (dialValue < -50) dialValue = -50;
    if (dialValue >  50) dialValue =  50;
    emit changePlayGainDial(dialValue);
}

void
PropertyBox::playGainDialChanged(int dialValue)
{
    QObject *obj = sender();

    PlayParameters *params = m_container->getPlayParameters();
    if (!params) return;

    float gain = float(pow(10, float(dialValue) / 20.0));

    if (params->getPlayGain() != gain) {
        PlayParameterRepository::EditCommand *command =
            new PlayParameterRepository::EditCommand(params);
        command->setPlayGain(gain);
        CommandHistory::getInstance()->addCommand(command, true, true);
    }

    updateContextHelp(obj);
}
    
void
PropertyBox::playPanChanged(float pan)
{
    int dialValue = int(lrint(pan * 50.0));
    if (dialValue < -50) dialValue = -50;
    if (dialValue >  50) dialValue =  50;
    emit changePlayPanDial(dialValue);
}

void
PropertyBox::playPanDialChanged(int dialValue)
{
    QObject *obj = sender();

    PlayParameters *params = m_container->getPlayParameters();
    if (!params) return;

    float pan = float(dialValue) / 50.f;
    if (pan < -1.f) pan = -1.f;
    if (pan >  1.f) pan =  1.f;

    if (params->getPlayPan() != pan) {
        PlayParameterRepository::EditCommand *command =
            new PlayParameterRepository::EditCommand(params);
        command->setPlayPan(pan);
        CommandHistory::getInstance()->addCommand(command, true, true);
    }

    updateContextHelp(obj);
}

void
PropertyBox::editPlayParameters()
{
    PlayParameters *params = m_container->getPlayParameters();
    if (!params) return;

    QString clip = params->getPlayClipId();

    PlayParameterRepository::EditCommand *command = 
        new PlayParameterRepository::EditCommand(params);
    
    QInputDialog *dialog = new QInputDialog(this);

    QDir dir(":/samples");
    QStringList clipFiles = dir.entryList(QStringList() << "*.wav", QDir::Files);

    QStringList clips;
    foreach (QString str, clipFiles) {
        clips.push_back(str.replace(".wav", ""));
    }
    dialog->setComboBoxItems(clips);

    dialog->setLabelText(tr("Set playback clip:"));

    QComboBox *cb = dialog->findChild<QComboBox *>();
    if (cb) {
        for (int i = 0; i < cb->count(); ++i) {
            if (cb->itemText(i) == clip) {
                cb->setCurrentIndex(i);
            }
        }
    }

    connect(dialog, SIGNAL(textValueChanged(QString)), 
            this, SLOT(playClipChanged(QString)));

    if (dialog->exec() == QDialog::Accepted) {
        QString newClip = dialog->textValue();
        command->setPlayClipId(newClip);
        CommandHistory::getInstance()->addCommand(command, true);
    } else {
        delete command;
        // restore in case we mucked about with the configuration
        // as a consequence of signals from the dialog
        params->setPlayClipId(clip);
    }

    delete dialog;
}

void
PropertyBox::playClipChanged(QString id)
{
    PlayParameters *params = m_container->getPlayParameters();
    if (!params) return;

    params->setPlayClipId(id);
}    

void
PropertyBox::layerVisibilityChanged(bool visible)
{
    if (m_showButton) m_showButton->setState(visible);
}

void
PropertyBox::mouseEnteredWidget()
{
    updateContextHelp(sender());
}

void
PropertyBox::updateContextHelp(QObject *o)
{
    QWidget *w = dynamic_cast<QWidget *>(o);
    if (!w) return;

    if (!m_container) return;
    QString cname = m_container->getPropertyContainerName();
    if (cname == "") return;

    QString wname = w->objectName();

    QString extraText;
    AudioDial *dial = dynamic_cast<AudioDial *>(w);
    if (dial) {
        double mv = dial->mappedValue();
        QString unit = "";
        if (dial->rangeMapper()) unit = dial->rangeMapper()->getUnit();
        if (unit != "") {
            extraText = tr(" (current value: %1%2)").arg(mv).arg(unit);
        } else {
            extraText = tr(" (current value: %1)").arg(mv);
        }
    }

    if (w == m_showButton) {
        emit contextHelpChanged(tr("Toggle Visibility of %1").arg(cname));
    } else if (w == m_playButton) {
        emit contextHelpChanged(tr("Toggle Playback of %1").arg(cname));
    } else if (wname == "") {
        return;
    } else if (dynamic_cast<QAbstractButton *>(w)) {
        emit contextHelpChanged(tr("Toggle %1 property of %2")
                                .arg(wname).arg(cname));
    } else {
        emit contextHelpChanged(tr("Adjust %1 property of %2%3")
                                .arg(wname).arg(cname).arg(extraText));
    }
}

void
PropertyBox::mouseLeftWidget()
{
    if (!(QApplication::mouseButtons() & Qt::LeftButton)) {
        emit contextHelpChanged("");
    }
}


