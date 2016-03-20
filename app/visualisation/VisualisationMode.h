#ifndef VISUALISATIONMODE_H
#define VISUALISATIONMODE_H

#include <QObject>
#include "QtilitiesCoreGui/IMode.h"
using namespace Qtilities::CoreGui::Interfaces;

namespace Praaline {

// Visualisation Mode Parameters
#define MODE_VISUALISATION_ID 996
const char * const CONTEXT_VISUALISATION_MODE = "Context.VisualisationMode";

// This structure stores private data used by the VisualisationMode class.
struct VisualisationModePrivateData;

class VisualisationMode : public QObject, public IMode {
    Q_OBJECT
    Q_INTERFACES(Qtilities::CoreGui::Interfaces::IMode)
public:
    VisualisationMode(QObject *parent = 0);
    ~VisualisationMode();

    // IObjectBase Implementation
    QObject* objectBase() { return this; }
    const QObject* objectBase() const { return this; }

    // IMode Implementation
    QWidget* modeWidget();
    void initializeMode();
    QIcon modeIcon() const;
    QString modeName() const;
    QString contextString() const { return CONTEXT_VISUALISATION_MODE; }
    QString contextHelpId() const { return QString(); }
    int modeID() const { return MODE_VISUALISATION_ID; }

signals:
    void modeIconChanged();
    void modeActivated();
    void modeDeactivated();

public slots:
    void modeManagerActiveModeChanged(int new_mode_id, int old_mode_id);
    void activateMode();

private:
    VisualisationModePrivateData *d;
};

}

#endif // VISUALISATIONMODE_H
