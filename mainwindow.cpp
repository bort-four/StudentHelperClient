#include <QHostAddress>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../StudentHelperServer/shquery.h"
#include "../StudentHelperServer/studenthelpercommon.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&_serverSocket, SIGNAL(readyRead()),
            this, SLOT(onReadyRead()));

//    _soket.connectToHost(QHostAddress("localhost"), 1234);
    _serverSocket.connectToHost(QHostAddress("127.0.0.1"), 1234);

    qDebug() << "wait for connection...";

    _serverSocket.waitForConnected();

    if (_serverSocket.state() == QTcpSocket::ConnectedState)
    {
        qDebug() << "connected";
    }
    else
        qDebug() << "some error:" << _serverSocket.errorString();
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onReadyRead()
{
    QTcpSocket *socketPtr = dynamic_cast<QTcpSocket *>(sender());

    if (socketPtr == nullptr)
        return;

    SHQueryBase *queryPtr = nullptr;

    try
    {
        queryPtr = SHQueryBase::fromQByteArray(socketPtr->readAll());

        // read content
        if (dynamic_cast<SHQContent *>(queryPtr) != nullptr)
        {
            SHQContent *contentQueryPtr = dynamic_cast<SHQContent *>(queryPtr);

            _content.setFileList(contentQueryPtr->getFileList());
            _content.setRootFolderPtr(contentQueryPtr->getRootFolderPtr());
            _content.debugOutput();
        }

        /*
        // read global file list
        if (dynamic_cast<SHQFileList *>(queryPtr) != nullptr)
        {
            SHQFileList *listQueryPtr = dynamic_cast<SHQFileList *>(queryPtr);

            for (auto file : listQueryPtr->getFileList())
            {
                File *filePtr = new File(file._fileName);
                filePtr->inputTagsFromString(file._tagString);

                _content.getFileList().append(filePtr);
            }
        }

        // read file list
        if (dynamic_cast<SHQFileTree *>(queryPtr) != nullptr)
        {
            SHQFileTree *treeQueryPtr = dynamic_cast<SHQFileTree *>(queryPtr);

            treeQueryPtr->setFileListPtr(_content.getFileList());

            //
        }
        */
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
