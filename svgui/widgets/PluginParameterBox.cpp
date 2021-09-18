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

#include "PluginParameterBox.h"

#include "AudioDial.h"

#include "plugin/PluginXml.h"
#include "plugin/RealTimePluginInstance.h" // for PortHint stuff

#include "base/RangeMapper.h"

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLayout>
#include <QLabel>

#include <iostream>
#include <string>

#include <cmath>

PluginParameterBox::PluginParameterBox(Vamp::PluginBase *plugin, QWidget *parent) :
    QFrame(parent),
    m_plugin(plugin),
    m_programCombo(0)
{
    m_layout = new QGridLayout;
    setLayout(m_layout);
    populate();
}

PluginParameterBox::~PluginParameterBox()
{
}

void
PluginParameterBox::populate()
{
    Vamp::PluginBase::ParameterList params = m_plugin->getParameterDescriptors();
    m_programs = m_plugin->getPrograms();

    m_params.clear();

    if (params.empty() && m_programs.empty()) {
        m_layout->addWidget
            (new QLabel(tr("This plugin has no adjustable parameters.")),
             0, 0);
    }

    int offset = 0;

    if (!m_programs.empty()) {

        std::string currentProgram = m_plugin->getCurrentProgram();

        m_programCombo = new QComboBox;
        m_programCombo->setMaxVisibleItems
            (int(m_programs.size() < 25 ? m_programs.size() : 20));

        for (int i = 0; in_range_for(m_programs, i); ++i) {
            m_programCombo->addItem(m_programs[i].c_str());
            if (m_programs[i] == currentProgram) {
                m_programCombo->setCurrentIndex(int(i));
            }
        }

        m_layout->addWidget(new QLabel(tr("Program")), 0, 0);
        m_layout->addWidget(m_programCombo, 0, 1, 1, 2);

        connect(m_programCombo, SIGNAL(currentIndexChanged(const QString &)),
                this, SLOT(programComboChanged(const QString &)));

        offset = 1;
    }

    for (int i = 0; in_range_for(params, i); ++i) {

        QString identifier = params[i].identifier.c_str();
        QString name = params[i].name.c_str();
        QString unit = params[i].unit.c_str();

        float min = params[i].minValue;
        float max = params[i].maxValue;
        float deft = params[i].defaultValue;
        float value = m_plugin->getParameter(params[i].identifier);

        int hint = PortHint::NoHint;
        RealTimePluginInstance *rtpi = dynamic_cast<RealTimePluginInstance *>
            (m_plugin);
        if (rtpi) {
            hint = rtpi->getParameterDisplayHint(i);
        }

        float qtz = 0.0;
        if (params[i].isQuantized) qtz = params[i].quantizeStep;

//        cerr << "PluginParameterBox: hint = " << hint << ", min = " << min << ", max = "
//                  << max << ", qtz = " << qtz << endl;

        std::vector<std::string> valueNames = params[i].valueNames;

        // construct an integer range

        int imin = 0, imax = 100;

        if (!(hint & PortHint::Logarithmic)) {
            if (qtz > 0.0) {
                imax = int(lrintf((max - min) / qtz));
            } else {
                qtz = (max - min) / 100.f;
            }
        }

        //!!! would be nice to ensure the default value corresponds to
        // an integer!

        QLabel *label = new QLabel(name);
        if (params[i].description != "") {
            label->setToolTip(QString("<qt>%1</qt>")
                              .arg(params[i].description.c_str())
                              .replace("\n", "<br>"));
        }
        m_layout->addWidget(label, i + offset, 0);

        ParamRec rec;
        rec.param = params[i];
        rec.dial = 0;
        rec.spin = 0;
        rec.check = 0;
        rec.combo = 0;
        
        if (params[i].isQuantized && !valueNames.empty()) {
            
            QComboBox *combobox = new QComboBox;
            combobox->setObjectName(identifier);
            for (unsigned int j = 0; j < valueNames.size(); ++j) {
                combobox->addItem(valueNames[j].c_str());
                if ((unsigned int)(lrintf(fabsf((value - min) / qtz))) == j) {
                    combobox->setCurrentIndex(j);
                }
            }
            connect(combobox, SIGNAL(activated(int)),
                    this, SLOT(dialChanged(int)));
            m_layout->addWidget(combobox, i + offset, 1, 1, 2);
            rec.combo = combobox;

        } else if (min == 0.0 && max == 1.0 && qtz == 1.0) {
            
            QCheckBox *checkbox = new QCheckBox;
            checkbox->setObjectName(identifier);
            checkbox->setCheckState(value < 0.5 ? Qt::Unchecked : Qt::Checked);
            connect(checkbox, &QCheckBox::stateChanged,
                    this, &PluginParameterBox::checkBoxChanged);
            m_layout->addWidget(checkbox, i + offset, 2);
            rec.check = checkbox;

        } else {
            
            AudioDial *dial = new AudioDial;
            dial->setObjectName(name);
            dial->setMinimum(imin);
            dial->setMaximum(imax);
            dial->setPageStep(1);
            dial->setNotchesVisible((imax - imin) <= 12);
//!!!            dial->setDefaultValue(lrintf((deft - min) / qtz));
//            dial->setValue(lrintf((value - min) / qtz));
            dial->setFixedWidth(32);
            dial->setFixedHeight(32);
            RangeMapper *rm = 0;
            if (hint & PortHint::Logarithmic) {
                rm = new LogRangeMapper(imin, imax, min, max, unit);
            } else {
                rm = new LinearRangeMapper(imin, imax, min, max, unit);
            }
            dial->setRangeMapper(rm);
            dial->setDefaultValue(rm->getPositionForValue(deft));
            dial->setValue(rm->getPositionForValue(value));
            dial->setShowToolTip(true);
            connect(dial, &QAbstractSlider::valueChanged,
                    this, &PluginParameterBox::dialChanged);
            m_layout->addWidget(dial, i + offset, 1);

            QDoubleSpinBox *spinbox = new QDoubleSpinBox;
            spinbox->setObjectName(identifier);
            spinbox->setMinimum(min);
            spinbox->setMaximum(max);
            spinbox->setSuffix(QString(" %1").arg(unit));
            if (qtz != 0) spinbox->setSingleStep(qtz);
            spinbox->setValue(value);
            spinbox->setDecimals(4);
            connect(spinbox, SIGNAL(valueChanged(double)),
                    this, SLOT(spinBoxChanged(double)));
            m_layout->addWidget(spinbox, i + offset, 2);
            rec.dial = dial;
            rec.spin = spinbox;
        }

        m_params[identifier] = rec;
        m_nameMap[name] = identifier;
    }
}

void
PluginParameterBox::dialChanged(int ival)
{
    QObject *obj = sender();
    QString identifier = obj->objectName();

    if (m_params.find(identifier) == m_params.end() &&
        m_nameMap.find(identifier) != m_nameMap.end()) {
        identifier = m_nameMap[identifier];
    }

    if (m_params.find(identifier) == m_params.end()) {
        cerr << "WARNING: PluginParameterBox::dialChanged: Unknown parameter \"" << identifier << "\"" << endl;
        return;
    }

    Vamp::PluginBase::ParameterDescriptor params = m_params[identifier].param;

    float min = params.minValue;
    float max = params.maxValue;

    float newValue;

    float qtz = 0.0;
    if (params.isQuantized) qtz = params.quantizeStep;

    AudioDial *ad = dynamic_cast<AudioDial *>(obj);
    
    if (ad && ad->rangeMapper()) {
        
        newValue = float(ad->mappedValue());
        if (newValue < min) newValue = min;
        if (newValue > max) newValue = max;
        if (qtz != 0.0) {
            ival = int(lrintf((newValue - min) / qtz));
            newValue = min + float(ival) * qtz;
        }

    } else {
        if (qtz == 0.f) {
            qtz = (max - min) / 100.f;
        }
        newValue = min + float(ival) * qtz;
    }

//    cerr << "PluginParameterBox::dialChanged: newValue = " << newValue << endl;

    QDoubleSpinBox *spin = m_params[identifier].spin;
    if (spin) {
        spin->blockSignals(true);
        spin->setValue(newValue);
        spin->blockSignals(false);
    }

//    cerr << "setting plugin parameter \"" << identifier << "\" to value " << newValue << endl;

    m_plugin->setParameter(identifier.toStdString(), newValue);

    updateProgramCombo();

    emit pluginConfigurationChanged(PluginXml(m_plugin).toXmlString());
}

void
PluginParameterBox::checkBoxChanged(int state)
{
    QObject *obj = sender();
    QString identifier = obj->objectName();

    if (m_params.find(identifier) == m_params.end() &&
        m_nameMap.find(identifier) != m_nameMap.end()) {
        identifier = m_nameMap[identifier];
    }

    if (m_params.find(identifier) == m_params.end()) {
        cerr << "WARNING: PluginParameterBox::checkBoxChanged: Unknown parameter \"" << identifier << "\"" << endl;
        return;
    }

    Vamp::PluginBase::ParameterDescriptor params = m_params[identifier].param;

    if (state) m_plugin->setParameter(identifier.toStdString(), 1.0);
    else m_plugin->setParameter(identifier.toStdString(), 0.0);

    updateProgramCombo();

    emit pluginConfigurationChanged(PluginXml(m_plugin).toXmlString());
}

void
PluginParameterBox::spinBoxChanged(double value)
{
    QObject *obj = sender();
    QString identifier = obj->objectName();

    if (m_params.find(identifier) == m_params.end() &&
        m_nameMap.find(identifier) != m_nameMap.end()) {
        identifier = m_nameMap[identifier];
    }

    if (m_params.find(identifier) == m_params.end()) {
        cerr << "WARNING: PluginParameterBox::spinBoxChanged: Unknown parameter \"" << identifier << "\"" << endl;
        return;
    }

    Vamp::PluginBase::ParameterDescriptor params = m_params[identifier].param;

    float min = params.minValue;
    float max = params.maxValue;

    float qtz = 0.0;
    if (params.isQuantized) qtz = params.quantizeStep;
    
    if (qtz > 0.0) {
        int step = int(lrintf(float(value - min) / qtz));
        value = min + float(step) * qtz;
    }

//    int imax = 100;
    
    if (qtz > 0.0) {
//        imax = lrintf((max - min) / qtz);
    } else {
        qtz = (max - min) / 100.f;
    }

    int ival = int(lrintf(float(value - min) / qtz));

    AudioDial *dial = m_params[identifier].dial;
    if (dial) {
        dial->blockSignals(true);
        if (dial->rangeMapper()) {
            dial->setMappedValue(value);
        } else {
            dial->setValue(ival);
        }
        dial->blockSignals(false);
    }

    cerr << "setting plugin parameter \"" << identifier << "\" to value " << value << endl;

    m_plugin->setParameter(identifier.toStdString(), float(value));

    updateProgramCombo();

    emit pluginConfigurationChanged(PluginXml(m_plugin).toXmlString());
}

void
PluginParameterBox::programComboChanged(const QString &newProgram)
{
    m_plugin->selectProgram(newProgram.toStdString());

    for (std::map<QString, ParamRec>::iterator i = m_params.begin();
         i != m_params.end(); ++i) {

        Vamp::PluginBase::ParameterDescriptor &param = i->second.param;
        float value = m_plugin->getParameter(param.identifier);

        if (i->second.spin) {
            i->second.spin->blockSignals(true);
            i->second.spin->setValue(value);
            i->second.spin->blockSignals(false);
        }

        if (i->second.dial) {

            float min = param.minValue;
            float max = param.maxValue;

            float qtz = 0.0;
            if (param.isQuantized) qtz = param.quantizeStep;

            if (qtz == 0.0) {
                qtz = (max - min) / 100.f;
            }

            i->second.dial->blockSignals(true);
            i->second.dial->setValue(int(lrintf(float(value - min) / qtz)));
            i->second.dial->blockSignals(false);
        }

        if (i->second.combo) {
            i->second.combo->blockSignals(true);
            i->second.combo->setCurrentIndex(int(lrintf(value)));
            i->second.combo->blockSignals(false);
        }

        if (i->second.check) {
            i->second.check->blockSignals(true);
            i->second.check->setCheckState(value < 0.5 ? Qt::Unchecked : Qt::Checked);
            i->second.check->blockSignals(false);
        }            
    }

    emit pluginConfigurationChanged(PluginXml(m_plugin).toXmlString());
}

void
PluginParameterBox::updateProgramCombo()
{
    if (!m_programCombo || m_programs.empty()) return;

    std::string currentProgram = m_plugin->getCurrentProgram();

    for (int i = 0; in_range_for(m_programs, i); ++i) {
        if (m_programs[i] == currentProgram) {
            m_programCombo->setCurrentIndex(i);
        }
    }
}


