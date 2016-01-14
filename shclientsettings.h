#ifndef SHCLIENTSETTINGS_H
#define SHCLIENTSETTINGS_H

#include <QObject>
#include <QString>


class SHClientSettings : public QObject
{
    Q_OBJECT

public:
    SHClientSettings();
    ~SHClientSettings();

    QString getServerHost() const;
    int getMaxCacheSize() const;

    void saveSettings() const;
    void restoreSettings();

signals:
    void serverHostChanged(QString);
    void maxCacheSizeChanged(int);

public slots:
    void setMaxCacheSize(int maxCacheSize);
    void setServerHost(const QString &serverHost);

private:
    QString _serverHost = "127.0.0.1";
    int _maxCacheSize = 50; // in Mb
};

#endif // SHCLIENTSETTINGS_H
