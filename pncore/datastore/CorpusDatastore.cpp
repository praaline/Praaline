#include "corpus/CorpusObject.h"
#include "CorpusRepository.h"
#include "CorpusDatastore.h"

namespace Praaline {
namespace Core {

CorpusDatastore::CorpusDatastore(CorpusRepository *repository, QObject *parent) :
    QObject(parent), m_repository(repository)
{
}


void CorpusDatastore::setRepository(CorpusObject *object)
{
    object->m_repository = this->m_repository;
}

} // namespace Core
} // namespace Praaline
