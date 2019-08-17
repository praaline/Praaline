#ifndef SCRIPTINGMODE_H
#define SCRIPTINGMODE_H

#include <QObject>
#include "QtilitiesCoreGui/IMode.h"
using namespace Qtilities::CoreGui::Interfaces;

namespace Praaline {

// Scripting Mode Parameters
#define MODE_SCRIPTING_ID 1006
const char * const CONTEXT_SCRIPTING_MODE = "Context.ScriptingMode";

// This structure stores private data used by the ScriptingMode class.
struct ScriptingModePrivateData;

class ScriptingMode : public QObject, public IMode {
    Q_OBJECT
    Q_INTERFACES(Qtilities::CoreGui::Interfaces::IMode)
public:
    ScriptingMode(QObject *parent = nullptr);
    ~ScriptingMode();

    // IObjectBase Implementation
    QObject* objectBase() { return this; }
    const QObject* objectBase() const { return this; }

    // IMode Implementation
    QWidget* modeWidget();
    void initializeMode();
    QIcon modeIcon() const;
    QString modeName() const;
    QString contextString() const { return CONTEXT_SCRIPTING_MODE; }
    QString contextHelpId() const { return QString(); }
    int modeID() const { return MODE_SCRIPTING_ID; }

signals:
    void modeIconChanged();
    void modeActivated();
    void modeDeactivated();

public slots:
    void modeManagerActiveModeChanged(int new_mode_id, int old_mode_id);
    void activateMode();

private:
    ScriptingModePrivateData *d;
};

}

#endif // SCRIPTINGMODE_H


