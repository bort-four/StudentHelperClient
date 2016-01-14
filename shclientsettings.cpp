#include <QSettings>

#include "shclientsettings.h"



SHClientSettings::SHClientSettings()
{
    restoreSettings();
}


SHClientSettings::~SHClientSettings()
{
    saveSettings();
}


void SHClientSettings::saveSettings() const
{
    QSettings settings("MMCS","StudentHelperClient");
    settings.clear();

    settings.setValue("ServerHost", getServerHost());
    settings.setValue("MaxCacheSize", getMaxCacheSize());
}

void SHClientSettings::restoreSettings()
{
    QSettings settings("MMCS","StudentHelperClient");

    setServerHost(settings.value("ServerHost", "127.0.0.1").toString());
    setMaxCacheSize(settings.value("MaxCacheSize", 50).toInt());
}



QString SHClientSettings::getServerHost() const
{
    return _serverHost;
}

void SHClientSettings::setServerHost(const QString &serverHost)
{
    _serverHost = serverHost;
    emit serverHostChanged(_serverHost);
}

int SHClientSettings::getMaxCacheSize() const
{
    return _maxCacheSize;
}


void SHClientSettings::setMaxCacheSize(int maxCacheSize)
{
    _maxCacheSize = maxCacheSize;
    emit maxCacheSizeChanged(_maxCacheSize);
}


