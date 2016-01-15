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

    // for settings page
    connect(ui->cacheSizeSlider,    SIGNAL(valueChanged(int)),
            ui->cacheSizeSpinBox,   SLOT(setValue(int)));

    connect(ui->cacheSizeSpinBox,   SIGNAL(valueChanged(int)),
            ui->cacheSizeSlider,    SLOT(setValue(int)));


    connect(&_settings,     SIGNAL(serverHostChanged(QString)),
            ui->hostLine,   SLOT(setText(QString)));

    connect(ui->hostLine,   SIGNAL(textChanged(QString)),
            &_settings,     SLOT(setServerHost(QString)));


    connect(&_settings,             SIGNAL(maxCacheSizeChanged(int)),
            ui->cacheSizeSlider,    SLOT(setValue(int)));

    connect(ui->cacheSizeSlider,    SIGNAL(valueChanged(int)),
            &_settings,             SLOT(setMaxCacheSize(int)));

    /*
<<<<<<< HEAD
=======
    connect(_browserWidget,     SIGNAL(printRequested(File*)),
            _shContentPtr,      SIGNAL(sendToPrint(File*))   );
    connect(_browserWidget,     SIGNAL(tagClicked(QString)),
            _searcherWidget,    SLOT(tagSearchInit(QString)) );
    connect(_browserWidget, SIGNAL(tagClicked(QString)),
            this,           SLOT(openSearchTab()) );
>>>>>>> c00f00b6008fefd71678917ffb69d62d062f0fa1
    */

    connect(ui->connectButton,  SIGNAL(clicked()),
            this,               SLOT(connectToServer()));

    _settings.restoreSettings();


    _shCache = new SHCache(_settings.getMaxCacheSize());

    connect(ui->cacheSizeSlider, SIGNAL(valueChanged(int)),
            _shCache,            SLOT(changeSize(int)));
    connect(ui->cacheCleanButton, SIGNAL(clicked(bool)),
            _shCache,             SLOT(clean()));


    ui->searchTab->setLayout(new QHBoxLayout);
    ui->printTab->setLayout(new QHBoxLayout);

    connect(&_serverReader, SIGNAL(hasReadyFrame()),
            this,           SLOT(onFrameIsReady()));

    connectToServer();
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
            SHQImage *imageQPtr = dynamic_cast<SHQImage *>(queryPtr);
            File* file = _shContentPtr->getFileList()[imageQPtr->getFileId()];
            file->setPixmap(imageQPtr->getImagePtr());
            _shCache->add(file);
        }

        // if there is image request
        if (dynamic_cast<SHQImageRequest *>(queryPtr) != nullptr)
        {
            SHQImageRequest *requestPtr = dynamic_cast<SHQImageRequest *>(queryPtr);
            SHQImage answer;

            answer.setFileId(requestPtr->getFileId());
            answer.setImagePtr(_shContentPtr->getFileList()[requestPtr->getFileId()]->getImage());

            _serverReader.writeData(answer.toQByteArray());
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


void SHClientWidget::connectToServer()
{
    if (_serverReader.getSocketPtr() != nullptr)
    {
        _serverReader.getSocketPtr()->close();
        delete _serverReader.getSocketPtr();
    }

    if (_browserWidget != nullptr)
        delete _browserWidget;

    if (_searcherWidget != nullptr)
        delete _searcherWidget;

    if (_printerWidget != nullptr)
        delete _printerWidget;

    // conection...
    QTcpSocket *socketPtr = new QTcpSocket();
    _serverReader = FrameReader(socketPtr);

    socketPtr->connectToHost(QHostAddress(_settings.getServerHost()), 4321);
    socketPtr->waitForConnected();

    if (socketPtr->state() == QTcpSocket::ConnectedState)
        ;
    else
        qDebug() << "Connection error:" << socketPtr->errorString();
}


void SHClientWidget::onContentEdited()
{
//    qDebug() << "Content edited";

    SHQContent query;
    query.setFileList(_shContentPtr->getFileList());
    query.setRootFolderPtr(_shContentPtr->getRootFolder());

    _serverReader.writeData(query.toQByteArray());
}


void SHClientWidget::onCurrFolderChanged()
{
    // request images for files in current folder
    FolderItem *folderPtr = _browserWidget->getCurrFolder();

    for (auto fileItemPtr : folderPtr->getFiles())
        if (fileItemPtr->getFilePtr()->getImage() == nullptr)
        {
            QString id = fileItemPtr->getFilePtr()->getUuid();
            QPixmap* pix = _shCache->get(id);
            if (pix != NULL)
            {
                int index = _shContentPtr->getFileList().indexOf(fileItemPtr->getFilePtr());
                _shContentPtr->getFileList()[index]->setPixmap(pix);
            }
            else
            {
                SHQImageRequest query;
                query.setFileId(_shContentPtr->getFileList().indexOf(fileItemPtr->getFilePtr()));

                _serverReader.writeData(query.toQByteArray());
            }
        }
}



StudentHelperContent *SHClientWidget::getSHContentPtr() const
{
    return _shContentPtr;
}



void SHClientWidget::setSHContentPtr(StudentHelperContent *shContentPtr)
{
    shContentPtr->getRootFolder()->setName("Все разделы");

    _shContentPtr = shContentPtr;
    _browserWidget = new FileBrowserWidget(_shContentPtr->getRootFolder());
    _browserWidget->setEdittingEnabled(true);
    _browserWidget->setPrintEnabled(true);
    ui->fileTab->layout()->addWidget(_browserWidget);

    _searcherWidget = new SearcherWidget(NULL, _shContentPtr);
    clearLayout(ui->searchTab->layout());
    ui->searchTab->layout()->addWidget(_searcherWidget);

    _printerWidget = new PrinterWidget(NULL, _shContentPtr);
    clearLayout(ui->printTab->layout());
    ui->printTab->layout()->addWidget(_printerWidget);

    connect(_browserWidget,     SIGNAL(printRequested(File*)),
            _shContentPtr,      SIGNAL(sendToPrint(File*))   );
    connect(_browserWidget,     SIGNAL(tagClicked(QString)),
            _searcherWidget,    SLOT(tagSearchInit(QString)) );
    connect(_browserWidget, SIGNAL(tagClicked(QString)),
            this,           SLOT(openSearchTab()) );

    connect(_browserWidget, SIGNAL(currFolderChanged()),
            this,           SLOT(onCurrFolderChanged()));

    connect(_shContentPtr,  SIGNAL(contentEdited()),
            this,           SLOT(onContentEdited()));

    onCurrFolderChanged();
}

