#include "cachingbase.h"
#include <QTcpSocket>
#include <QApplication>
#include <QDateTime>

#include <QTextEdit>

SHCache::SHCache(int max_size_MB) : QObject()
{
    m_mem_limit = max_size_MB * 1024 * 1024;

    // Creating cache directory, if not exist
    QDir dir(QApplication::applicationDirPath());
    if(!dir.cd("Cache"))
    {
        dir.mkdir("Cache");
    }

    // Stored files memory value evaluating
    m_mem_counter = getMemoryValue();

    // Clearing redundund memory, if is
    while(m_mem_counter >= m_mem_limit)
    {
        removeOldest();
    }
}

int SHCache::getMemoryValue()
{
    QDir dir(QApplication::applicationDirPath() + "/Cache");

    QFileInfoList info_list = dir.entryInfoList();

    int counter = 0;
    for(int i = 0; i < info_list.size(); ++i)
    {
        counter += info_list[i].size();
    }

    return counter;
}

QPixmap* SHCache::get(const QString &file_id)
{
    QString name = QApplication::applicationDirPath() + "/Cache";
    QDir dir(name);
    QFileInfoList info_list = dir.entryInfoList();

    for(QFileInfoList::iterator it = info_list.begin(); it != info_list.end(); ++it)
    {
        if(it->baseName() == file_id)
        {
            return new QPixmap(it->absoluteFilePath());
        }
    }
    return NULL;
}

void SHCache::removeOldest()
{
    QDir dir(QApplication::applicationDirPath() + "/Cache");
    QFileInfoList info_list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);

    if (info_list.empty())
    {
        return;
    }

    int index = 0;
    QDateTime earlier_data(info_list[index].created());
    for(int i = 1; i < info_list.size(); ++i)
    {
        QDateTime time = info_list[i].created();
        if (time < earlier_data)
        {
            earlier_data = time;
            index = i;
        }
    }

    QFileInfo& info = info_list[index];
    while (dir.remove(info.fileName()))
    {
        m_mem_counter -= info.size();
    }
}

void SHCache::add(const File* file)
{
    QString name = QApplication::applicationDirPath() + "/Cache/" + file->getUuid() + ".jpg";
    file->getImage()->save(name, NULL, 100);

    QFile created_file(name);
    m_mem_counter += created_file.size();

    while (m_mem_counter >= m_mem_limit)
    {
        removeOldest();
    }
}

void SHCache::changeSize(const int sizeMB)
{
    m_mem_limit = sizeMB * 1024 * 1024;
    while(m_mem_counter >= m_mem_limit)
    {
        removeOldest();
    }
}

void SHCache::clean()
{
    QDir dir(QApplication::applicationDirPath() + "/Cache");
    QFileInfoList info_list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);

    for(int i = 0; i < info_list.size(); ++i)
    {
        dir.remove(info_list[i].fileName());
    }

    m_mem_counter = 0;
}
