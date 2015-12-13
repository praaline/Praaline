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

#ifndef _PLUGIN_PARAMETER_DIALOG_H_
#define _PLUGIN_PARAMETER_DIALOG_H_

#include <QDialog>

#include "base/Window.h"

#include <vamp-hostsdk/PluginBase.h>

class PluginParameterBox;
class QWidget;
class QPushButton;
class QLabel;
class QGroupBox;
class QComboBox;
class QCheckBox;

/**
 * A dialog for editing the parameters of a given plugin, using a
 * PluginParameterBox.  This dialog does not contain any mechanism for
 * selecting the plugin in the first place.  Note that the dialog
 * directly modifies the parameters of the plugin, so they will remain
 * modified even if the dialog is then cancelled.
 */

class PluginParameterDialog : public QDialog
{
    Q_OBJECT
    
public:
    PluginParameterDialog(Vamp::PluginBase *, QWidget *parent = 0);
    ~PluginParameterDialog();

    void setChannelArrangement(int sourceChannels,
                               int targetChannels,
                               int defaultChannel);

    void setOutputLabel(QString output, QString description);

    void setMoreInfoUrl(QString url);

    void setShowProcessingOptions(bool showWindowSize,
                                  bool showFrequencyDomainOptions);

    void setCandidateInputModels(const QStringList &names,
                                 QString defaultName);
    void setShowSelectionOnlyOption(bool show);

    Vamp::PluginBase *getPlugin() { return m_plugin; }

    int getChannel() const { return m_channel; }

    QString getInputModel() const;
    bool getSelectionOnly() const;

    //!!! merge with PluginTransform::ExecutionContext

    void getProcessingParameters(int &blockSize) const;
    void getProcessingParameters(int &stepSize, int &blockSize,
                                 WindowType &windowType) const;

    int exec();

signals:
    void pluginConfigurationChanged(QString);
    void inputModelChanged(QString);

protected slots:
    void channelComboChanged(int);
    void blockSizeComboChanged(const QString &);
    void incrementComboChanged(const QString &);
    void windowTypeChanged(WindowType type);
    void advancedToggled();
    void moreInfo();
    void setAdvancedVisible(bool);
    void inputModelComboChanged(int);
    void selectionOnlyChanged(int);
    void dialogAccepted();

protected:
    Vamp::PluginBase *m_plugin;

    int m_channel;
    int m_stepSize;
    int m_blockSize;

    WindowType m_windowType;
    PluginParameterBox *m_parameterBox;

    QLabel *m_outputLabel;
    QLabel *m_outputValue;
    QLabel *m_outputDescription;
    QLabel *m_outputSpacer;

    QPushButton *m_moreInfo;
    QString m_moreInfoUrl;

    QGroupBox *m_channelBox;
    bool m_haveChannelBoxData;

    QGroupBox *m_windowBox;
    bool m_haveWindowBoxData;

    QGroupBox *m_inputModelBox;
    QComboBox *m_inputModels;
    QCheckBox *m_selectionOnly;
    QStringList m_inputModelList;
    QString m_currentInputModel;
    bool m_currentSelectionOnly;

    QPushButton *m_advancedButton;
    QWidget *m_advanced;
    bool m_advancedVisible;
};

#endif

