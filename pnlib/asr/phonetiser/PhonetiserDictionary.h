#ifndef PHONETISERDICTIONARY_H
#define PHONETISERDICTIONARY_H

#include <QObject>

class PhonetiserDictionary : public QObject
{
    Q_OBJECT
public:
    explicit PhonetiserDictionary(QObject *parent = nullptr);

signals:

public slots:
};

#endif // PHONETISERDICTIONARY_H