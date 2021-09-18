/****************************************************************************
**
** Copyright (c) 2009-2013, Jaco Naudé
**
** This file is part of Qtilities.
**
** For licensing information, please see
** http://jpnaude.github.io/Qtilities/page_licensing.html
**
****************************************************************************/

#include "HelpMode.h"
#include "HelpModeConfig.h"
#include "ui_HelpModeConfig.h"

#include <QtilitiesApplication>
#include <QtilitiesCoreGuiConstants>

using namespace Qtilities::CoreGui::Constants;

HelpModeConfig::HelpModeConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HelpModeConfig)
{
    ui->setupUi(this);
}

HelpModeConfig::~HelpModeConfig() {
    delete ui;
}

QIcon HelpModeConfig::configPageIcon() const {
    return QIcon(Praaline::ICON_HELP_MODE);
}

QWidget* HelpModeConfig::configPageWidget() {
    return this;
}

QString HelpModeConfig::configPageTitle() const {
    return tr(qti_config_page_HELP);
}

Qtilities::Core::QtilitiesCategory HelpModeConfig::configPageCategory() const {
    if (IConfigPage::configPageCategory().isEmpty())
        return QtilitiesCategory(tr(qti_config_page_DEFAULT_CAT));
    else
        return IConfigPage::configPageCategory();
}

void HelpModeConfig::configPageApply() {
    // Only refresh when registered files changed:
    if (files_widget.stringList() == HELP_MANAGER->registeredFiles())
        return;

    disconnect(HELP_MANAGER,&Qtilities::CoreGui::HelpManager::registeredFilesChanged,this,&HelpModeConfig::handleFilesChanged);
    HELP_MANAGER->clearRegisteredFiles(false);
    HELP_MANAGER->registerFiles(files_widget.stringList());
    connect(HELP_MANAGER,&Qtilities::CoreGui::HelpManager::registeredFilesChanged,this, &HelpModeConfig::handleFilesChanged);
    HELP_MANAGER->writeSettings();
}

void HelpModeConfig::configPageInitialize() {
    if (ui->widgetFilesHolder->layout())
        delete ui->widgetFilesHolder->layout();

    QHBoxLayout* layout = new QHBoxLayout(ui->widgetFilesHolder);
    layout->addWidget(&files_widget);
    layout->setMargin(0);
    files_widget.show();
    files_widget.setListType(StringListWidget::FilePaths);
    files_widget.setFileOpenDialogFilter("Help Files (*.qch)");

    connect(HELP_MANAGER,&Qtilities::CoreGui::HelpManager::registeredFilesChanged,this, &HelpModeConfig::handleFilesChanged);
    handleFilesChanged(HELP_MANAGER->registeredFiles());
}

void HelpModeConfig::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void HelpModeConfig::handleFilesChanged(const QStringList& files) {
    // All paths in resources, thus starting with :/ are relative.
    QStringList read_only_paths;
    foreach (const QString& file, files) {
        if (file.startsWith(":/"))
            read_only_paths << HelpManager::formatFileName(file);
    }
    files_widget.setStringList(files);
    files_widget.setNonRemovableStringList(read_only_paths);
}
