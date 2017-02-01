/****************************************************************************
**
** Copyright 2010-2013, CSIR
** Author: JP Naude, jpnaude@csir.co.za
**
****************************************************************************/

#include "HelpBrowser.h"

#include <QtGui>
#include <QTextBrowser>
#include <QSslSocket>

#include <QtilitiesCoreGui>
using namespace QtilitiesCoreGui;


HelpTextBrowser::HelpTextBrowser(QHelpEngine* helpEngine, QWidget* parent) :
    QTextBrowser(parent), helpEngine(helpEngine)
{
}

QVariant HelpTextBrowser::loadResource(int type, const QUrl &name)
{
    if (name.scheme() == "qthelp")
        return QVariant(helpEngine->fileData(name));
    // else
    return QTextBrowser::loadResource(type, name);
}


struct HelpBrowserData {
    HelpBrowserData() :
        browser(0),
        btnSearch(0),
        btnBack(0),
        btnForward(0),
        btnReload(0),
        locationBar(0),
        txtLocationEdit(0)
    {}

    QPointer<HelpTextBrowser> browser;
    QPushButton* btnSearch;
    QToolButton* btnBack;
    QToolButton* btnForward;
    QPushButton* btnReload;
    QWidget* locationBar;
    QLineEdit* txtLocationEdit;
    SearchBoxWidget* searchBoxWidget;
    QString errorMsg;
};

HelpBrowser::HelpBrowser(QHelpEngine* helpEngine, const QUrl &url, QWidget* parent) :
    QWidget(parent), d(new HelpBrowserData)
{
    // Init d pointers
    d->btnBack = new QToolButton;
    d->btnForward = new QToolButton;
    d->btnReload = new QPushButton;
    d->locationBar = new QWidget;
    d->txtLocationEdit = new QLineEdit;
    d->txtLocationEdit->setReadOnly(true);

    d->browser = new HelpTextBrowser(helpEngine, this);
    d->browser->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    loadUrl(url);

    //----------------------
    // Set Button Attributes
    //----------------------
    d->btnBack->setText(tr("Back"));
    d->btnBack->setArrowType(Qt::LeftArrow);
    d->btnBack->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->btnForward->setText(tr("Forward"));
    d->btnForward->setArrowType(Qt::RightArrow);
    d->btnForward->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->btnReload->setText(tr("Reload"));
    d->btnReload->setIcon(QIcon(qti_icon_REFRESH_16x16));

    // Searching
    SearchBoxWidget::SearchOptions search_options = 0;
    search_options |= SearchBoxWidget::CaseSensitive;
    SearchBoxWidget::ButtonFlags button_flags = 0;
    button_flags |= SearchBoxWidget::HideButtonUp;
    button_flags |= SearchBoxWidget::NextButtons;
    button_flags |= SearchBoxWidget::PreviousButtons;
    d->searchBoxWidget = new SearchBoxWidget(search_options,SearchBoxWidget::SearchOnly,button_flags);
    connect(d->searchBoxWidget,SIGNAL(btnClose_clicked()),SLOT(handleSearchClose()));
    d->searchBoxWidget->hide();

    d->btnSearch = new QPushButton;
    d->btnSearch->setIcon(QIcon(qti_icon_FIND_16x16));
    connect(d->btnSearch,SIGNAL(clicked()),SLOT(showSearchBox()));
    connect(d->searchBoxWidget,SIGNAL(searchStringChanged(QString)),SLOT(handleSearchStringChanged(QString)));
    connect(d->searchBoxWidget,SIGNAL(searchOptionsChanged()),SLOT(handleSearchOptionsChanged()));

    // Config SearchBar Layout
    if (d->locationBar->layout())
        delete d->locationBar->layout();

    QHBoxLayout* Hlayout = new QHBoxLayout;
    Hlayout->setMargin(0);
    Hlayout->addWidget(d->btnBack);
    Hlayout->addWidget(d->btnForward);
    Hlayout->addWidget(d->btnReload);
    Hlayout->addWidget(d->btnSearch);
    Hlayout->addWidget(d->txtLocationEdit);
    d->locationBar->setLayout(Hlayout);
    d->locationBar->setFixedHeight(25);

    // Config Documentation Tab Layout
    if (layout())
        delete layout();

    QVBoxLayout* Vlayout = new QVBoxLayout(this);
    Vlayout->setMargin(0);
    Vlayout->setSpacing(0);
    Vlayout->addWidget(d->locationBar);
    Vlayout->addWidget(d->searchBoxWidget);
    Vlayout->addWidget(d->browser);

    //----------------------
    // Connect SearchBar to QWebEngineView
    //----------------------
//    connect(d->btnBack,SIGNAL(clicked()),d->web_view,SLOT(back()));
//    connect(d->btnForward,SIGNAL(clicked()),d->web_view,SLOT(forward()));
//    connect(d->btnReload,SIGNAL(clicked()),d->web_view,SLOT(reload()));
//    connect(d->txtLocationEdit,SIGNAL(returnPressed()),this,SLOT(handle_loadUrl()));
//    connect(d->web_view,SIGNAL(urlChanged(QUrl)),this,SLOT(handle_urlChanged(QUrl)));
//    connect(d->web_view,SIGNAL(loadFinished(bool)),this,SLOT(handle_finished(bool)));
    d->browser->show();

    // The error message:
    d->errorMsg = QString("<html><body><h1>Error, could not load the page you requested.</h1></body></html>");
}

HelpBrowser::~HelpBrowser() {
    delete d;
}

void HelpBrowser::loadUrl(const QUrl &url) {
    d->browser->setSource(url);
}

HelpTextBrowser *HelpBrowser::helpTextBrowser() {
    return d->browser;
}

void HelpBrowser::handle_loadUrl( ){
    // connect(d->web_view,SIGNAL(loadProgress(int)),SLOT(handleLoadProgress(int)));
    loadUrl(QUrl(d->txtLocationEdit->text()));
    d->locationBar->setEnabled(false);

    QUrl new_url(d->txtLocationEdit->text());

    if (new_url.scheme() == "https") {
        #ifdef Q_OS_WIN
        if (!QSslSocket::supportsSsl()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Failed To Load Webpage"));
            msgBox.setText(tr("Your system and/or Qt installation does not support websites using SSL (thus https://).\n\n"
                              "The page you are trying to load uses SSL and therefore cannot be loaded:\n") + d->browser->source().toString());
            msgBox.exec();
            LOG_ERROR("Error while loading secure page at " + d->browser->source().toString() + ". SSL is not supported.");
            return;
        }
        #else
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Failed To Load Webpage"));
        msgBox.setText(tr("Your system and/or Qt installation does not seem to support websites using SSL (thus https://).\n\n"
                          "The page you are trying to load uses SSL and therefore cannot be loaded:\n") + d->browser->source().toString());
        msgBox.exec();
        LOG_ERROR("Error while loading secure page at " + d->browser->source().toString() + ". SSL is not supported.");
        return;
        #endif
    }

    emit newUrlEntered(new_url);
}

void HelpBrowser::handle_urlChanged(QUrl url){
    d->txtLocationEdit->setText(url.toString());
}

void HelpBrowser::handle_finished(bool ok){
    if (!d->browser)
        return;

    if (!ok) {
        if (!d->browser->source().toString().isEmpty())
            LOG_ERROR("Error while loading page at " + d->browser->source().toString());
    }

    d->locationBar->setEnabled(true);
}

void HelpBrowser::showSearchBox() {
    d->searchBoxWidget->setEditorFocus();

    if (!d->searchBoxWidget->isVisible())
        d->searchBoxWidget->show();
    else
        d->searchBoxWidget->hide();
}

void HelpBrowser::handleSearchStringChanged(const QString& search_string) {
//    QWebEnginePage::FindFlags find_flags = 0;

//    if (d->searchBoxWidget->caseSensitive())
//        find_flags |= QWebEnginePage::FindCaseSensitively;

//    //find_flags |= QWebPage::FindWrapsAroundDocument;
//    //find_flags |= QWebPage::HighlightAllOccurrences;

//    d->web_view->findText(search_string,find_flags);
}

void HelpBrowser::handleSearchOptionsChanged() {
//    d->web_view->findText(QString());
//    handleSearchStringChanged(d->searchBoxWidget->currentSearchString());
}

void HelpBrowser::handleSearchForward() {
//    handleSearchStringChanged(d->searchBoxWidget->currentSearchString());
}

void HelpBrowser::handleSearchBackwards() {
//    QWebEnginePage::FindFlags find_flags = 0;

//    if (d->searchBoxWidget->caseSensitive())
//        find_flags |= QWebEnginePage::FindCaseSensitively;

//    //find_flags |= QWebPage::FindWrapsAroundDocument;
//    //find_flags |= QWebEnginePage::HighlightAllOccurrences;
//    find_flags |= QWebEnginePage::FindBackward;

//    d->web_view->findText(d->searchBoxWidget->currentSearchString(),find_flags);
}

void HelpBrowser::handleSearchClose() {
//    d->searchBoxWidget->close();
//    d->web_view->findText(QString());
}
