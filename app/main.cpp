#include <QApplication>
#include <QTranslator>
#include "PraalineMainWindow.h"

int main(int argc, char *argv[])
{
    // Initialize application object, using Qtilities instead of QApplication
    QtilitiesApplication a(argc, argv);
    QtilitiesApplication::setOrganizationName("praaline.org");
    QtilitiesApplication::setOrganizationDomain("Praaline");
    QtilitiesApplication::setApplicationName("Praaline");
    QtilitiesApplication::setApplicationVersion("0.2");

    // Create a QtilitiesMainWindow. This window will show Praaline's different modes (e.g. Corpus, Annotation...)
    QtilitiesMainWindow mainWindow(QtilitiesMainWindow::ModesLeft);
    QtilitiesApplication::setMainWindow(&mainWindow);
    PraalineMainWindow praalineMainWindow(&mainWindow);

    // RUN THE APPLICATION
    praalineMainWindow.initialise();
    int result = a.exec();
    praalineMainWindow.finalise();
    return result;
}
