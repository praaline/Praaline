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

#ifndef HELP_MODE_H
#define HELP_MODE_H

#include <IMode.h>

#include <QMainWindow>
#include <QPointer>
#include <QWebEngineUrlSchemeHandler>
#include <QBuffer>

namespace Ui {
    class HelpMode;
}

class QHelpEngine;
class QHelpEngineCore;
class QUrl;
class QWebEngineView;

namespace Qtilities {
    namespace Plugins {
        namespace Help {
            using namespace Qtilities::CoreGui::Interfaces;

            class qti_private_QHelpUrlSchemeHandler : public QWebEngineUrlSchemeHandler
            {
                Q_OBJECT
            public:
                qti_private_QHelpUrlSchemeHandler(QHelpEngineCore* helpEngine, QObject *parent = Q_NULLPTR);

                virtual void requestStarted(QWebEngineUrlRequestJob *request);

            protected:
                QPointer<QHelpEngineCore>   d_help_engine;

            private:
                Q_DISABLE_COPY(qti_private_QHelpUrlSchemeHandler)
            };

            // Help Mode Parameters
            #define MODE_HELP_ID                   997
            const char * const CONTEXT_HELP_MODE   = "Context.HelpMode";

            /*!
              \struct HelpModeData
              \brief The HelpModeData class stores private data used by the HelpMode class.
             */
            struct HelpModeData;

            /*!
            \class HelpMode
            \brief An implementation of Qtilities::CoreGui::Interfaces::IMode which make a GUI frontend for the \p HELP_MANAGER available as a mode in an application.
              */
            class HelpMode : public QMainWindow, public IMode {
                Q_OBJECT
                Q_INTERFACES(Qtilities::CoreGui::Interfaces::IMode)
            public:
                HelpMode(QWidget *parent = 0);
                ~HelpMode();
                bool eventFilter(QObject *object, QEvent *event);

                //! Initializes the help mode.
                void initiallize();

                // --------------------------------
                // IObjectBase Implementation
                // --------------------------------
                QObject* objectBase() { return this; }
                const QObject* objectBase() const { return this; }

                // --------------------------------------------
                // IMode Implementation
                // --------------------------------------------
                QWidget* modeWidget();
                void initializeMode();
                QIcon modeIcon() const;
                bool setModeIcon(QIcon icon);
                QString modeName() const;
                QString contextString() const { return CONTEXT_HELP_MODE; }
                QString contextHelpId() const { return QString(); }
                int modeID() const { return MODE_HELP_ID; }

            public slots:
                //! Toggles the visibility of the dynamic help dock widget.
                void toggleDock(bool toggle);
                //! Handles a new help widget. This function makes the necessary connections.
                void handleNewHelpWidget(QWidget* widget);
                //! Display the page at \p url.
                void handleUrl(const QUrl& url);
                //! Handle requests from the help manager to display the url.
                void handleUrlRequest(const QUrl& url, bool ensure_visible);
                //! Handle home page changed.
                void handleHomePageChanged(const QUrl& url);

            private:
                HelpModeData* d;
            };
        }
    }
}

#endif // HELP_MODE_H
