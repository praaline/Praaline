#ifndef HELP_MODE_H
#define HELP_MODE_H

#include <QObject>
#include "QtilitiesCoreGui/IMode.h"
using namespace Qtilities::CoreGui::Interfaces;

namespace Praaline {

// Help Mode Parameters
#define MODE_HELP_ID 1007
const char * const CONTEXT_HELP_MODE = "Context.HelpMode";
const char * const ICON_HELP_MODE = ":/icons/modes/help_mode_48x48.png";

// This structure stores private data used by the HelpMode class.
struct HelpModePrivateData;

class HelpMode : public QObject, public IMode {
    Q_OBJECT
    Q_INTERFACES(Qtilities::CoreGui::Interfaces::IMode)
public:
    HelpMode(QObject *parent = 0);
    ~HelpMode();

    // IObjectBase Implementation
    QObject* objectBase() { return this; }
    const QObject* objectBase() const { return this; }

    // IMode Implementation
    QWidget* modeWidget();
    void initializeMode();
    QIcon modeIcon() const;
    QString modeName() const;
    QString contextString() const { return CONTEXT_HELP_MODE; }
    QString contextHelpId() const { return QString(); }
    int modeID() const { return MODE_HELP_ID; }

signals:
    void modeIconChanged();
    void modeActivated();
    void modeDeactivated();

public slots:
    void modeManagerActiveModeChanged(int new_mode_id, int old_mode_id);
    void activateMode();

private:
    HelpModePrivateData *d;
};

}

#endif // HELP_MODE_H
