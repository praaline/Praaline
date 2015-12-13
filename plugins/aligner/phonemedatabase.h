#ifndef PHONEMEDATABASE_H
#define PHONEMEDATABASE_H

#include <QString>
#include <QList>

class PhonemeDatabase
{
public:
    PhonemeDatabase();
    ~PhonemeDatabase();

    bool createHMMDefsBootstrap(const QString &filename);
private:
    QList<QString> m_phonemes;

};

#endif // PHONEMEDATABASE_H
