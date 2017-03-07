#include <QString>
#include <QDir>
#include "FileDatastore.h"

namespace Praaline {
namespace Core {

FileDatastore::FileDatastore(QObject *parent) :
    QObject(parent)
{
}

void FileDatastore::setBasePath(const QString &path)
{
    m_basePath = QDir(path).absolutePath();
}

QString FileDatastore::getRelativeToBasePath(const QString &absoluteFilePath) const
{
    if (m_basePath.isEmpty())
        return absoluteFilePath;
    return QDir(QDir(m_basePath).canonicalPath()).relativeFilePath(absoluteFilePath);
}

} // namespace Core
} // namespace Praaline
