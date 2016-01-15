#ifndef CACHINGBASE
#define CACHINGBASE

#include <QObject>
#include <QDir>
#include <QFile>
#include <QPixmap>
#include "../StudentHelperServer/filetreeitem.h"
#include "../StudentHelperServer/shquery.h"
#include "../StudentHelperServer/studenthelpercontent.h"


// Class providing methods for local cache service
class SHCache : public QObject
{
    Q_OBJECT

private:
    int getMemoryValue();

    void removeOldest();

public:
    SHCache(int max_size);

    // Try to find file in cache.
    QPixmap *get(const QString& file_id);

    // Add file to cache.
    void add(const File* file);

public slots:
    void changeSize(int sizeMB);

    void clean();

private:

    // Count of currently stored images.
    int m_mem_counter;

    // Memory limit for stored images.
    int m_mem_limit;
};

#endif // CACHINGBASE
