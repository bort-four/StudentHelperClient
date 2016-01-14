#ifndef SH_CLIENT_H
#define SH_CLIENT_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTcpSocket>

#include "filebrowserwidget.h"
#include "searcherwidget.h"
#include "printerwidget.h"
#include "shclientsettings.h"
#include "cachingbase.h"
#include "../StudentHelperServer/studenthelpercontent.h"
#include "../StudentHelperServer/shquery.h"

namespace Ui {
class SHClientWidget;
}


class SHClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SHClientWidget(QWidget *parent = 0);
    ~SHClientWidget();

    StudentHelperContent *getSHContentPtr() const;
    void setSHContentPtr(StudentHelperContent *getSHContentPtr);


public slots:
    void openSearchTab();

private slots:
    void onCurrFolderChanged();
    void onFrameIsReady();
    void connectToServer();
    void onContentEdited();

private:
    Ui::SHClientWidget *ui;
    StudentHelperContent* _shContentPtr = nullptr;
    FileBrowserWidget* _browserWidget = nullptr;
    SearcherWidget* _searcherWidget = nullptr;
    PrinterWidget* _printerWidget = nullptr;

    SHClientSettings _settings;
    FrameReader _serverReader;
    SHCache* _shCache;
};



#endif // SH_CLIENT_H
