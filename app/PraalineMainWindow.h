#ifndef PRAALINEMAINWINDOW_H
#define PRAALINEMAINWINDOW_H

#include <QObject>
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

struct PraalineMainWindowData;

class PraalineMainWindow : public QObject
{
    Q_OBJECT
public:
    explicit PraalineMainWindow(QtilitiesMainWindow *mainWindow, QObject *parent = 0);
    ~PraalineMainWindow();
    void initialise();
    void finalise();
    void selectLightPalette();
    void selectDarkPalette();

signals:

private slots:
    void showHelp();
    void showAbout();
    void showWebsite();
    void showKeyReference();
    void showActivityLog();
    void showUnitConverter();
    void toggleColourPalette();

private:
    QtilitiesMainWindow *m_mainWindow;
    PraalineMainWindowData *d;

    void setupMenuBar();
    void setupFileMenu();
    void setupEditMenu();
    void setupViewMenu();
    void setupCorpusMenu();
    void setupAnnotationMenu();
    void setupVisualisationMenu();
    void setupPlaybackMenu();
    void setupWindowMenu();
    void setupHelpMenu();
};

#endif // PRAALINEMAINWINDOW_H
