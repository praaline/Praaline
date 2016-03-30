#ifndef PRAATSESSION_H
#define PRAATSESSION_H

#include <QString>
#include <pthread.h>
#include "../sys/praatlib.h"

class PRAAT_LIB_EXPORT PraatSession
{
public:
    static QString last_error();
    static void clear_error();
    static void set_error(const char* err);
    static void set_melder_error();

private:
    static pthread_mutex_t m_mutex;
    static QString m_lastError;
    PraatSession();
};

#endif // PRAATSESSION_H
