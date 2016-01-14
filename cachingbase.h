#ifndef CACHINGBASE
#define CACHINGBASE

#include <QDir>
#include <QFile>
#include <QPixmap>
#include "../StudentHelperServer/filetreeitem.h"
#include "../StudentHelperServer/shquery.h"
#include "../StudentHelperServer/studenthelpercontent.h"


// Class providing methods for access
// to server database through local cache.
class SHCache
{
private:
    int getRecordsCount();

    quint64 getMaxID();

    int contains(const QString& name);

    QPixmap* get(const QString &name);

    QString getFormat(const QString& name);

    void removeOldest();

    void newRecord(const QString& name);

    void remove(const QString& name);

public:
    SHCache(int max_size, StudentHelperContent* content, FrameReader *reader);

    // Try to find file in cache, if no then download this file,
    // put it to cache, and return.
    QPixmap *getPixmap(const File* file);

    // Delete file from cache if there,
    // and send deliting query to server.
    void deletePixmap(FileItem *file_item);

    void resetReader(FrameReader* new_reader);

private:

    // Count of currently stored images.
    int m_counter;

    // ID counter
    quint64 m_id_counter;

    // Maximum count of stored images.
    int m_limit;

    // File containing information about stored data.
    QFile m_cache_info;

    FrameReader* m_reader;

    // Pointer to object containing metadata of database.
    StudentHelperContent* m_content;
};

#endif // CACHINGBASE
