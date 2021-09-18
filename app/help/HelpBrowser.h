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

#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include <IMode>
#include <QTextBrowser>
#include <QHelpEngine>

/*!
  \struct HelpBrowserData
  \brief The HelpBrowserData struct stores private data used by the HelpBrowser class.
 */
struct HelpBrowserData;

/*!
  \struct HelpTextBrowser
  \brief A QTextBrowser subclass capable of using Qt Help files.
 */
class HelpTextBrowser : public QTextBrowser
{
public:
    HelpTextBrowser(QHelpEngine* helpEngine, QWidget *parent = nullptr);
    QVariant loadResource (int type, const QUrl& name);
private:
    QHelpEngine* helpEngine;
};

/*!
\class HelpBrowser
\brief A very simple browser wrapper around QTextBrowser used to display help pages.
  */
class HelpBrowser : public QWidget
{
    Q_OBJECT
    public:
        HelpBrowser(QHelpEngine *helpEngine, const QUrl &url = QUrl(), QWidget *parent = nullptr);
        ~HelpBrowser();

        //! Loads the specified url.
        void loadUrl(const QUrl &url = QUrl());

        //! Returns a pointer to the contained HelpTextBrowser.
        HelpTextBrowser* helpTextBrowser();

    private slots:
        void handle_loadUrl();
        void handle_urlChanged(QUrl url);
        void handle_finished(bool ok);
        void showSearchBox();
        void handleSearchStringChanged(const QString& search_string);
        void handleSearchOptionsChanged();
        void handleSearchForward();
        void handleSearchBackwards();
        void handleSearchClose();

    signals:
        void newUrlEntered(const QUrl& url);

    private:
        HelpBrowserData* d;
};

#endif // HELPBROWSER_H
