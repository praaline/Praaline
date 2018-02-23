#ifndef DISMOCONFIGURATION_H
#define DISMOCONFIGURATION_H

#include <QString>

class DisMoConfiguration
{
public:
    DisMoConfiguration();

    static QString resourcesPath();
    static QString filePathTokeniserKeepTogether(const QString &language);
    static QString filePathDictionary(const QString &language);
    static QString filePathMWUDictionary(const QString &language);

    static QString filePathCRFTest();
};

#endif // DISMOCONFIGURATION_H
