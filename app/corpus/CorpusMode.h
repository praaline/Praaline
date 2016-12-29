#ifndef CORPUSMODE_H
#define CORPUSMODE_H

#include <QObject>
#include "QtilitiesCoreGui/IMode.h"
using namespace Qtilities::CoreGui::Interfaces;

namespace Praaline {

// Corpus Mode Parameters
#define MODE_CORPUS_ID 1001
const char * const CONTEXT_CORPUS_MODE = "Context.CorpusMode";

// This structure stores private data used by the CorpusMode class.
struct CorpusModePrivateData;

class CorpusMode : public QObject, public IMode {
    Q_OBJECT
    Q_INTERFACES(Qtilities::CoreGui::Interfaces::IMode)
public:
    CorpusMode(QObject *parent = 0);
    ~CorpusMode();

    // IObjectBase Implementation
    QObject* objectBase() { return this; }
    const QObject* objectBase() const { return this; }

    // IMode Implementation
    QWidget* modeWidget();
    void initializeMode();
    QIcon modeIcon() const;
    QString modeName() const;
    QString contextString() const { return CONTEXT_CORPUS_MODE; }
    QString contextHelpId() const { return QString(); }
    int modeID() const { return MODE_CORPUS_ID; }

signals:
    void modeIconChanged();
    void modeActivated();
    void modeDeactivated();

public slots:
    void modeManagerActiveModeChanged(int new_mode_id, int old_mode_id);
    void activateMode();

private:
    CorpusModePrivateData *d;
};

}

#endif // CORPUSMODE_H
