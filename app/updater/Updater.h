#ifndef UPDATER_H
#define UPDATER_H

// Copyright (c) 2014-2016 Alex Spataru <alex_spataru@outlook.com>

#include <QUrl>
#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "QSimpleUpdater.h"

class Downloader;

/**
 * \brief Downloads and interprests the update definition file
 */
class Updater : public QObject
{
    Q_OBJECT

signals:
    void checkingFinished (const QString& url);
    void downloadFinished (const QString& url, const QString& filepath);
    void appcastDownloaded (const QString& url, const QByteArray& data);

public:
    Updater();
    ~Updater();

    QString url() const;
    QString openUrl() const;
    QString changelog() const;
    QString moduleName() const;
    QString downloadUrl() const;
    QString platformKey() const;
    QString moduleVersion() const;
    QString latestVersion() const;
    QString userAgentString() const;

    bool customAppcast() const;
    bool notifyOnUpdate() const;
    bool notifyOnFinish() const;
    bool updateAvailable() const;
    bool downloaderEnabled() const;
    bool useCustomInstallProcedures() const;

public slots:
    void checkForUpdates();
    void setUrl (const QString& url);
    void setModuleName (const QString& name);
    void setNotifyOnUpdate (const bool notify);
    void setNotifyOnFinish (const bool notify);
    void setUserAgentString (const QString& agent);
    void setModuleVersion (const QString& version);
    void setDownloaderEnabled (const bool enabled);
    void setPlatformKey (const QString& platformKey);
    void setUseCustomAppcast (const bool customAppcast);
    void setUseCustomInstallProcedures (const bool custom);

private slots:
    void onReply (QNetworkReply* reply);
    void setUpdateAvailable (const bool available);

private:
    bool compare (const QString& x, const QString& y);

private:
    QString m_url;
    QString m_userAgentString;

    bool m_customAppcast;
    bool m_notifyOnUpdate;
    bool m_notifyOnFinish;
    bool m_updateAvailable;
    bool m_downloaderEnabled;

    QString m_openUrl;
    QString m_platform;
    QString m_changelog;
    QString m_moduleName;
    QString m_downloadUrl;
    QString m_moduleVersion;
    QString m_latestVersion;

    Downloader* m_downloader;
    QNetworkAccessManager* m_manager;
};

#endif // UPDATER_H
