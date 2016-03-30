#include <QString>
#include <pthread.h>
#include "../sys/praatlib.h"
#include "../sys/melder.h"

#include "PraatSession.h"

pthread_mutex_t PraatSession::m_mutex = PTHREAD_MUTEX_INITIALIZER;
QString PraatSession::m_lastError = QString();

PraatSession::PraatSession()
{
}


QString PraatSession::last_error() {
    QString ret;
    pthread_mutex_lock(&m_mutex);
    ret = m_lastError;
    pthread_mutex_unlock(&m_mutex);
    return ret;
}

void PraatSession::clear_error() {
    pthread_mutex_lock(&m_mutex);
    m_lastError = QString();
    pthread_mutex_unlock(&m_mutex);
}

void PraatSession::set_error(const char* err) {
    pthread_mutex_lock(&m_mutex);
    m_lastError = QString(err);
    pthread_mutex_unlock(&m_mutex);
}

 void PraatSession::set_melder_error() {
    char32* melderErr = Melder_getError();
    if(melderErr != NULL) {
        set_error(Melder_peek32to8(melderErr));
    }
}
