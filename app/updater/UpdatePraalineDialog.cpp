#include "UpdatePraalineDialog.h"
#include "ui_UpdatePraalineDialog.h"

// Simple updater
#include "updater/QSimpleUpdater.h"

struct UpdatePraalineDialogData {
    QSimpleUpdater* updater;
};

static const QString PRAALINE_DEFS_URL = "https://www.praaline.org/updates/praaline.json";

UpdatePraalineDialog::UpdatePraalineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdatePraalineDialog), d(new UpdatePraalineDialogData())
{
    ui->setupUi(this);
    // QSimpleUpdater is a singleton
    d->updater = QSimpleUpdater::getInstance();
    // Check updates slot
    connect(ui->commandCheckForUpdates, &QAbstractButton::clicked, this, &UpdatePraalineDialog::checkForUpdates);
    // Update changelog when update finishes
    connect (d->updater, &QSimpleUpdater::checkingFinished, this, &UpdatePraalineDialog::updateChangelog);
}

UpdatePraalineDialog::~UpdatePraalineDialog()
{
    delete ui;
    delete d;
}

void UpdatePraalineDialog::checkForUpdates()
{
     d->updater->checkForUpdates(PRAALINE_DEFS_URL);
}

void UpdatePraalineDialog::updateChangelog(const QString &url)
{
    if (url == PRAALINE_DEFS_URL) {
        if (!d->updater->getUpdateAvailable(url)) {
            ui->changelogText->setText(QString("Praaline is up to date.\n"
                                               "Current version: %1")
                                       .arg(d->updater->getModuleVersion(url)));
        } else {
            ui->changelogText->setText(d->updater->getChangelog(url));
        }
    }
}
