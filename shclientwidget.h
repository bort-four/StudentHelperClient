#ifndef SH_CLIENT_H
#define SH_CLIENT_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTcpSocket>

#include "filebrowserwidget.h"
#include "searcherwidget.h"
#include "printerwidget.h"
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

private:
    void initiaize();

    Ui::SHClientWidget *ui;
    StudentHelperContent* _shContentPtr = nullptr;
    FileBrowserWidget* _browserWidget = nullptr;
    SearcherWidget* _searcherWidget = nullptr;
    PrinterWidget* _printerWidget = nullptr;
    FrameReader _serverReader;
};



#endif // SH_CLIENT_H