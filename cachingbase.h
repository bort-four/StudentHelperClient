#ifndef CACHINGBASE
#define CACHINGBASE

#include <QCache>
#include "../StudentHelperServer/filetreeitem.h"
#include "../StudentHelperServer/shquery.h"
#include "../StudentHelperServer/studenthelpercontent.h"

// Class providing methods for access
// to server database through local cache.
class SHCache
{
public:
    SHCache(StudentHelperContent* content, int max_size = 100);

    // Try to find file in cache, if no then download this file,
    // put it to cache, and return.
    File* getFile(const File* file, FrameReader& reader);

    // Try to find every file of list in cache,
    // if no then download this file, put it to cache and return.
    QList<File*> getFileList(const QList<File*> &qlist, FrameReader& reader);

    // Delete file from cache if there,
    // and send deliting query to server.
    void deleteFile(FileItem *file_item, FrameReader& reader);

    // Try to find file in cache and edit it,
    // and send editinging query to server.
    File* editFileTags(const File* file, const QString& tag_string, FrameReader& reader);

private:
    // Standadrd cache class using as basis of files storing.
    QCache<QString,File> m_cache;

    // Pointer to object containing metadata of database.
    StudentHelperContent* m_content;
};

#endif // CACHINGBASE
