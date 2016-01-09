#include "cachingbase.h"
#include <QPixmap>
#include <QTcpSocket>


SHCache::SHCache(StudentHelperContent* content, int max_size)
{
    m_cache.setMaxCost(max_size);
    m_content = content;
}


File* SHCache::getFile(const File* file, FrameReader& reader)
{
    quint64 id = m_content->getFileId(file);

    if (m_cache.contains(file->getFullName()))
    {
        return m_cache.object(file->getFullName());
    }
    else
    {
        QTcpSocket* socket = const_cast<QTcpSocket*>(reader.getSocketPtr());

        // Query to server...
        SHQImageRequest req;
        req.setFileId(m_content->getFileId(file));
        reader.writeData(req.toQByteArray());

        // Pause for network interacton...
        if (!socket->waitForReadyRead())
        {
            qDebug() << "Error: Server don't reply to current query";
        }

        // Answer data is already adapted in background slot,
        // so we just get element of main list.
        File* result_file = m_content->getFileList().at(id);

        // Make file copy for cache...
        File* inserted_file = new File(result_file->getFullName());
        inserted_file->inputTagsFromString(result_file->getTagString());
        inserted_file->setPixmap(new QPixmap(result_file->getImage()->copy()));
        m_cache.insert(result_file->getFullName(), inserted_file);

        return result_file;
    }
}


QList<File*> SHCache::getFileList(const QList<File*>& qlist, FrameReader& reader)
{
    QList<File*> result;
    for(QList<File*>::const_iterator it = qlist.begin(); it != qlist.end(); ++it)
    {
        result.push_back(SHCache::getFile(*it, reader));
    }
    return result;
}


void SHCache::deleteFile(FileItem* file_item, FrameReader& reader)
{
    const File* file = file_item->getFilePtr();

    // Deleting from cahce if last link...
    if (file->getLinkCount() == 1)
    {
        // Deleting cache record if there...
        m_cache.remove(file->getFullName());
    }

    // Deleting file in local storage...
    FolderItem* folder = file_item->getParent();
    folder->removeChild(file_item);

    quint64 id = m_content->getFileId(file);
    m_content->getFileList().removeAt(id);

    // Query to server...
    SHQContent req;
    req.setFileList(m_content->getFileList());
    reader.writeData(req.toQByteArray());
}


File* SHCache::editFileTags(const File* file, const QString& tag_string, FrameReader& reader)
{
    // Editing of cache data if there...
    if (m_cache.contains(file->getFullName()))
    {
        File* cache_file = m_cache.object(file->getFullName());
        if (cache_file != NULL)
        {
            cache_file->inputTagsFromString(tag_string);
        }
    }

    // Editing of local data...
    quint64 id = m_content->getFileId(file);
    File* edited_file = m_content->getFileList().at(id);
    edited_file->inputTagsFromString(tag_string);

    // Query to server...
    SHQContent req;
    req.setFileList(m_content->getFileList());
    reader.writeData(req.toQByteArray());

    return edited_file;
}
