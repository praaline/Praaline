#ifndef WELCOMEMODE_H
#define WELCOMEMODE_H

#include <QObject>
#include "QtilitiesCoreGui/IMode.h"
using namespace Qtilities::CoreGui::Interfaces;

namespace Praaline {

// Welcome Mode Parameters
#define MODE_WELCOME_ID 100
const char * const CONTEXT_WELCOME_MODE = "Context.WelcomeMode";

// This structure stores private data used by the WelcomeMode class.
struct WelcomeModePrivateData;

class WelcomeMode : public QObject, public IMode {
    Q_OBJECT
    Q_INTERFACES(Qtilities::CoreGui::Interfaces::IMode)
public:
    WelcomeMode(QObject *parent = 0);
    ~WelcomeMode();

    // IObjectBase Implementation
    QObject* objectBase() { return this; }
    const QObject* objectBase() const { return this; }

    // IMode Implementation
    QWidget* modeWidget();
    void initializeMode();
    QIcon modeIcon() const;
    QString modeName() const;
    QString contextString() const { return CONTEXT_WELCOME_MODE; }
    QString contextHelpId() const { return QString(); }
    int modeID() const { return MODE_WELCOME_ID; }

signals:
    void modeIconChanged();
    void modeActivated();
    void modeDeactivated();

public slots:
    void modeManagerActiveModeChanged(int new_mode_id, int old_mode_id);
    void activateMode();

private:
    WelcomeModePrivateData *d;
};

} // namespace Praaline

#endif // WELCOMEMODE_H
