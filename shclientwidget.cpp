#include <QtCore>
#include <QStandardItemModel>
#include <QLabel>
#include <QHostAddress>

#include "shclientwidget.h"
#include "ui_shclientwidget.h"
#include "filebrowserwidget.h"
#include "printcompositor.h"
#include "../StudentHelperServer/studenthelpercommon.h"


SHClientWidget::SHClientWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SHClientWidget)
{
    ui->setupUi(this);

    // conection...
    QTcpSocket *socketPtr = new QTcpSocket();
    _serverReader.setSocketPtr(socketPtr);

//    connect(socketPtr,  SIGNAL(readyRead()),
//            this,       SLOT(onReadyRead()));

    connect(&_serverReader, SIGNAL(hasReadyFrame()),
            this,           SLOT(onFrameIsReady()));

    socketPtr->connectToHost(QHostAddress("127.0.0.1"), 1234);
    socketPtr->waitForConnected();

    if (socketPtr->state() != QTcpSocket::ConnectedState)
        qDebug() << "Connection error:" << socketPtr->errorString();
}


void SHClientWidget::initiaize()
{
    _searcherWidget = new SearcherWidget(NULL, _shContentPtr);
    ui->searchTab->setLayout(new QHBoxLayout);
    ui->searchTab->layout()->addWidget(_searcherWidget);

    _printerWidget = new PrinterWidget(NULL, _shContentPtr);
    ui->printTab->setLayout(new QHBoxLayout);
    ui->printTab->layout()->addWidget(_printerWidget);

//    connect(_browserWidget,     SIGNAL(printRequested(File*)),
//            _stHelperPtr,       SIGNAL(sendToPrint(File*))   );
    connect(_browserWidget,     SIGNAL(tagClicked(QString)),
            _searcherWidget,    SLOT(tagSearchInit(QString)) );
    connect(_browserWidget, SIGNAL(tagClicked(QString)),
            this,           SLOT(openSearchTab()) );

    connect(_browserWidget, SIGNAL(currFolderChanged()),
            this,           SLOT(onCurrFolderChanged()));

    onCurrFolderChanged();
}


SHClientWidget::~SHClientWidget()
{
    delete ui;
}


void SHClientWidget::openSearchTab()
{
    ui->tabWidget->setCurrentWidget(ui->searchTab);
}


void SHClientWidget::onFrameIsReady()
{
//    qDebug() << "Frame is ready";

    SHQueryBase *queryPtr = nullptr;
    QByteArray data = _serverReader.getFrameData();

    try
    {
        queryPtr = SHQueryBase::fromQByteArray(data);

        // read content
        if (dynamic_cast<SHQContent *>(queryPtr) != nullptr)
        {
            SHQContent *contentQueryPtr = dynamic_cast<SHQContent *>(queryPtr);
            StudentHelperContent *contentPtr = new StudentHelperContent(this);

            contentPtr->setFileList(contentQueryPtr->getFileList());
            contentPtr->setRootFolderPtr(contentQueryPtr->getRootFolderPtr());

            setSHContentPtr(contentPtr);
        }

        // read image
        if (dynamic_cast<SHQImage *>(queryPtr) != nullptr)
        {
//            return; // !!!!!!!!!

            SHQImage *imageQPtr = dynamic_cast<SHQImage *>(queryPtr);
            _shContentPtr->getFileList()[imageQPtr->getFileId()]
                    ->setPixmap(imageQPtr->getImagePtr());
        }
    }
    catch (SHException exc)
    {
        qDebug() << exc.getMsg();
    }
    catch (QString str)
    {
        qDebug() << str;
    }

    if (queryPtr != nullptr)
        delete queryPtr;
}


void SHClientWidget::onCurrFolderChanged()
{
//    qDebug() << "Current folder changed";

    // request images for files in current folder
    FolderItem *folderPtr = _browserWidget->getCurrFolder();

    for (auto fileItemPtr : folderPtr->getFiles())
        if (fileItemPtr->getFilePtr()->getImage() == nullptr)
        {
            SHQImageRequest query;
            query.setFileId(_shContentPtr->getFileList().indexOf(fileItemPtr->getFilePtr()));

            _serverReader.writeData(query.toQByteArray());

//            _serverSocket.write(query.toQByteArray());
//            return;

//            qDebug() << _shContentPtr->getFileList().indexOf(fileItemPtr->getFilePtr());
        }
}



StudentHelperContent *SHClientWidget::getSHContentPtr() const
{
    return _shContentPtr;
}


void SHClientWidget::setSHContentPtr(StudentHelperContent *shContentPtr)
{
    _shContentPtr = shContentPtr;
    _browserWidget = new FileBrowserWidget(_shContentPtr->getRootFolder());
    _browserWidget->setEdittingEnabled(true);
    _browserWidget->setPrintEnabled(true);
    ui->fileTab->layout()->addWidget(_browserWidget);

    initiaize();
}
