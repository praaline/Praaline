#ifndef QUERYMODE_H
#define QUERYMODE_H

#include <QObject>
#include "QtilitiesCoreGui/IMode.h"
using namespace Qtilities::CoreGui::Interfaces;

namespace Praaline {

// Query Mode Parameters
#define MODE_QUERY_ID 1004
const char * const CONTEXT_QUERY_MODE = "Context.QueryMode";

// This structure stores private data used by the CorpusMode class.
struct QueryModePrivateData;

class QueryMode : public QObject, public IMode {
    Q_OBJECT
    Q_INTERFACES(Qtilities::CoreGui::Interfaces::IMode)
public:
    QueryMode(QObject *parent = 0);
    ~QueryMode();

    // IObjectBase Implementation
    QObject* objectBase() { return this; }
    const QObject* objectBase() const { return this; }

    // IMode Implementation
    QWidget* modeWidget();
    void initializeMode();
    QIcon modeIcon() const;
    QString modeName() const;
    QString contextString() const { return CONTEXT_QUERY_MODE; }
    QString contextHelpId() const { return QString(); }
    int modeID() const { return MODE_QUERY_ID; }

signals:
    void modeIconChanged();
    void modeActivated();
    void modeDeactivated();

public slots:
    void modeManagerActiveModeChanged(int new_mode_id, int old_mode_id);
    void activateMode();

private:
    QueryModePrivateData *d;
};

}

#endif // QUERYMODE_H
