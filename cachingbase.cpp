#include "cachingbase.h"
#include <QTcpSocket>
#include <QApplication>

SHCache::SHCache(int max_size, StudentHelperContent* content)
{
    m_limit = max_size;
    m_content = content;

    QDir dir(QApplication::applicationDirPath());
    if(!dir.cd("Cache"))
    {
        dir.mkdir("Cache");
    }

    m_cache_info.setFileName(QApplication::applicationDirPath() + "/Cache/log.txt");
    m_counter = getRecordsCount();
    m_id_counter = getMaxID();

    /*
    QFile a(QApplication::applicationDirPath() + "/Cache/test_output.txt");
    a.open(QIODevice::Append);
    QTextStream ss(&a);
    ss << m_counter << "\r\n";
    ss << m_id_counter << "\r\n";
    a.close();
    */
}

int SHCache::getRecordsCount()
{
    int counter = 0;
    m_cache_info.open(QIODevice::ReadOnly);
    QTextStream stream(&m_cache_info);
    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        if (!line.isEmpty())
        {
            ++counter;
        }
    }
    m_cache_info.close();
    return counter;
}

quint64 SHCache::getMaxID()
{
    m_cache_info.open(QIODevice::ReadOnly);
    QTextStream stream(&m_cache_info);
    QString line;
    while (!stream.atEnd())
    {
        line = stream.readLine();
    }
    m_cache_info.close();

    if (line.isEmpty())
    {
        return 0;
    }
    QTextStream line_stream(&line);
    quint64 id;
    line_stream >> id;
    return id;
}

int SHCache::contains(const QString& name)
{
    m_cache_info.open(QIODevice::ReadOnly);
    QTextStream stream(&m_cache_info);
    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        if (line.isEmpty())
        {
            continue;
        }
        QTextStream line_stream(&line);

        quint64 id;
        line_stream >> id;

        QString file_name;
        line_stream >> file_name;

        if (file_name == name)
        {
            return id;
        }
    }
    m_cache_info.close();
    return -1;
}

QPixmap* SHCache::get(const QString& name)
{
    m_cache_info.open(QIODevice::ReadOnly);
    QTextStream stream(&m_cache_info);
    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        if (line.isEmpty())
        {
            continue;
        }
        QTextStream line_stream(&line);

        quint64 file_id;
        line_stream >> file_id;

        QString file_name;
        line_stream >> file_name;

        if (file_name == name)
        {
            QString format = getFormat(name);
            QString path = QApplication::applicationDirPath()   +
                           "/Cache/" + QString::number(file_id) +
                           "." + format;
            m_cache_info.close();
            return new QPixmap(path);
        }
    }
    m_cache_info.close();
    return NULL;
}

QString SHCache::getFormat(const QString& name)
{
    QString format;
    int i = name.length() - 1;
    while(name[i] != '.')
    {
        format.push_front(name[i]);
        --i;
    }
    return format;
}

void SHCache::removeOldest()
{

    m_cache_info.open(QIODevice::ReadOnly);
    if (m_cache_info.atEnd())
    {
        m_cache_info.close();
        return;
    }

    --m_counter;

    QTextStream stream(&m_cache_info);

    QString line = stream.readLine();
    QTextStream line_stream(&line);
    quint64 id;
    QString file_path, pixfile_name;
    line_stream >> id >> file_path;

    pixfile_name = QApplication::applicationDirPath() + "/Cache/" +
                   QString::number(id) + "." + getFormat(file_path);

    QFile file(pixfile_name);
    file.remove();

    QFile new_log(QApplication::applicationDirPath() + "/Cache/temp.txt");
    new_log.open(QIODevice::Append);
    QTextStream in_stream(&new_log);

    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        in_stream << line << "\r\n";
    }

    m_cache_info.close();
    new_log.close();

    m_cache_info.remove();
    new_log.copy(QApplication::applicationDirPath() + "/Cache/log.txt");
    new_log.remove();
}

void SHCache::newRecord(const QString& name)
{
    ++m_counter;
    ++m_id_counter;
    m_cache_info.open(QIODevice::Append);
    QTextStream stream(&m_cache_info);
    stream << name << "\r\n";
    m_cache_info.close();
}

void SHCache::remove(const QString& name)
{
    m_cache_info.open(QIODevice::ReadOnly);
    if (m_cache_info.atEnd())
    {
        m_cache_info.close();
        return;
    }

    --m_counter;

    QTextStream stream(&m_cache_info);

    QFile new_log(QApplication::applicationDirPath() + "/Cache/temp.txt");
    new_log.open(QIODevice::Append);
    QTextStream in_stream(&new_log);

    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        QTextStream data_stream(&line);
        int id;
        QString file_path;
        data_stream >> id >> file_path;

        if (file_path == name)
        {
            QString del_name = QApplication::applicationDirPath() +
                               "/Cache/" + QString::number(id)    +
                               "." + getFormat(file_path);
            QFile file(del_name);
            file.remove();
        }
        else
        {
            in_stream << line << "\r\n";
        }
    }

    m_cache_info.close();
    new_log.close();

    m_cache_info.remove();
    new_log.copy(QApplication::applicationDirPath() + "/Cache/log.txt");
    new_log.remove();
}

QPixmap* SHCache::getPixmap(const File* file, FrameReader& reader)
{
    QPixmap* pix = get(file->getFullName());
    if (pix != NULL)
    {
        return pix;
    }
    else
    {
        int id = m_content->getFileId(file);

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

        // Answer data is already got in background slot,
        // so we just get element of main list.
        File* result_file = m_content->getFileList().at(id);

        // Make file copy for cache...
        QPixmap* new_record = result_file->getImage();
        QString format = getFormat(file->getFullName());
        QString record_name = QApplication::applicationDirPath() +
                              "/Cache/" + QString::number(m_id_counter) +
                              "." + format;
        new_record->save(record_name);

        if (m_counter == m_limit)
        {
            removeOldest();
        }
        newRecord(result_file->getFullName());

        return new QPixmap(record_name);
    }
}

void SHCache::deletePixmap(FileItem* file_item, FrameReader& reader)
{
    const File* file = file_item->getFilePtr();

    // Deleting from cahce if last link...
    if (file->getLinkCount() == 1)
    {
        // Deleting cache record if there...
        remove(file->getFullName());
    }

    // Deleting file in local storage...
    FolderItem* folder = file_item->getParent();
    folder->removeChild(file_item);

    int id = m_content->getFileId(file);
    m_content->getFileList().removeAt(id);

    // Query to server...
    SHQContent req;
    req.setFileList(m_content->getFileList());
    reader.writeData(req.toQByteArray());
}
