#ifndef ANNOTATIONMODE_H
#define ANNOTATIONMODE_H

#include <QObject>
#include "QtilitiesCoreGui/IMode.h"
using namespace Qtilities::CoreGui::Interfaces;

namespace Praaline {

// Annotation Mode Parameters
#define MODE_ANNOTATION_ID 1002
const char * const CONTEXT_ANNOTATION_MODE = "Context.AnnotationMode";

// This structure stores private data used by the AnnotationMode class.
struct AnnotationModePrivateData;

class AnnotationMode : public QObject, public IMode {
    Q_OBJECT
    Q_INTERFACES(Qtilities::CoreGui::Interfaces::IMode)
public:
    AnnotationMode(QObject *parent = 0);
    ~AnnotationMode();

    // IObjectBase Implementation
    QObject* objectBase() { return this; }
    const QObject* objectBase() const { return this; }

    // IMode Implementation
    QWidget* modeWidget();
    void initializeMode();
    QIcon modeIcon() const;
    QString modeName() const;
    QString contextString() const { return CONTEXT_ANNOTATION_MODE; }
    QString contextHelpId() const { return QString(); }
    int modeID() const { return MODE_ANNOTATION_ID; }

signals:
    void modeIconChanged();
    void modeActivated();
    void modeDeactivated();

public slots:
    void modeManagerActiveModeChanged(int new_mode_id, int old_mode_id);
    void activateMode();

private:
    AnnotationModePrivateData *d;
};

}

#endif // ANNOTATIONMODE_H
