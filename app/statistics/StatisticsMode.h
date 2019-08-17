#ifndef STATISTICSMODE_H
#define STATISTICSMODE_H

#include <QObject>
#include "QtilitiesCoreGui/IMode.h"
using namespace Qtilities::CoreGui::Interfaces;

namespace Praaline {

// Statistics Mode Parameters
#define MODE_STATISTICS_ID 1005
const char * const CONTEXT_STATISTICS_MODE = "Context.StatisticsMode";

// This structure stores private data used by the StatisticsMode class.
struct StatisticsModePrivateData;

class StatisticsMode : public QObject, public IMode {
    Q_OBJECT
    Q_INTERFACES(Qtilities::CoreGui::Interfaces::IMode)
public:
    StatisticsMode(QObject *parent = nullptr);
    ~StatisticsMode();

    // IObjectBase Implementation
    QObject* objectBase() { return this; }
    const QObject* objectBase() const { return this; }

    // IMode Implementation
    QWidget* modeWidget();
    void initializeMode();
    QIcon modeIcon() const;
    QString modeName() const;
    QString contextString() const { return CONTEXT_STATISTICS_MODE; }
    QString contextHelpId() const { return QString(); }
    int modeID() const { return MODE_STATISTICS_ID; }

signals:
    void modeIconChanged();
    void modeActivated();
    void modeDeactivated();

public slots:
    void modeManagerActiveModeChanged(int new_mode_id, int old_mode_id);
    void activateMode();

private:
    StatisticsModePrivateData *d;
};

}

#endif // STATISTICSMODE_H
