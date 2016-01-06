#include <QSpacerItem>
#include <QToolButton>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileDialog>
#include <QDebug>

#include "filebrowserwidget.h"
#include "ui_filebrowserwidget.h"
#include "ui_filewidget.h"
#include "ui_folderwidget.h"
#include "filedialog.h"


// //// Service functions

void clearLayout(QLayout *layoutPtr)
{
    while (layoutPtr->itemAt(0) != NULL)
    {
        QLayoutItem* item = layoutPtr->itemAt(0);

        if (item->widget())
            delete item->widget();
        else if (item->layout())
            delete item->layout();
    }
}



void setMouseTrackingRecursive(QWidget *widgetPtr, bool value)
{
    widgetPtr->setMouseTracking(value);

    foreach (QObject* objPtr, widgetPtr->children()) {
        if (dynamic_cast<QWidget*>(objPtr))
            setMouseTrackingRecursive(dynamic_cast<QWidget*>(objPtr), value);
    }
}

// /////////////////////////



FileBrowserWidget::FileBrowserWidget(FolderItem* rootFolderPtr, QWidget *parent)
    : QWidget(parent), ui(new Ui::FileBrowserWidget)
    , _rootFolderPtr(NULL), _currFolderPtr(NULL), _currFileWgPtr(NULL)
    , _rootFolderVisible(false), _enableEdit(false), _enablePrint(true), _mouseHit(false)
{
    ui->setupUi(this);
    setRootFolder(rootFolderPtr);
}

FileBrowserWidget::~FileBrowserWidget()
{
    delete ui;
}


FolderItem *FileBrowserWidget::getRootFolder()
{
    return _rootFolderPtr;
}

FolderItem *FileBrowserWidget::getCurrFolder()
{
    return _currFolderPtr;
}

FileWiget *FileBrowserWidget::getCurrFileWidget()
{
    return _currFileWgPtr;
}

void FileBrowserWidget::setCurrFileWidget(FileWiget *wgPtr)
{
    if (wgPtr != NULL && _widgets.indexOf(wgPtr) == -1)
        throw QString("FileBrowserWidget::setCurrFileWidget(): invalid widget");

    _currFileWgPtr = wgPtr;
    emit currFileWidgetChanged(_currFileWgPtr);
}

bool FileBrowserWidget::getRootFolderVisible() const
{
    return _rootFolderVisible;
}

void FileBrowserWidget::setRootFolderVisible(bool visible)
{
    _rootFolderVisible = visible;
}

void FileBrowserWidget::setRootFolder(FolderItem *folderPtr)
{
    _rootFolderPtr = folderPtr;
    setCurrFolder(folderPtr);
}

void FileBrowserWidget::setCurrFolder(FolderItem *folderPtr)
{
    _currFolderPtr = folderPtr;
    _widgets.clear();
    QLayout* layoutPtr = ui->itemLayout;
    clearLayout(layoutPtr);
    clearLayout(ui->pathLayout);

    setCurrFileWidget(NULL);
    ui->spacerWidget->setHidden(false);
    ui->topPanel->setHidden(_currFolderPtr == _rootFolderPtr && !getEdittingEnabled());
    updateControlsVisible();

    if (_currFolderPtr == NULL) return;

    for (int itemNum = 0; itemNum < _currFolderPtr->getChildCount(); ++itemNum)
    {
        FileTreeItem *itemPtr = _currFolderPtr->getChild(itemNum);
        FileTreeWidget *wgPtr;

        if (itemPtr->isFolder())
        {
            FolderWidget* folderPtr = new FolderWidget(itemPtr->toFolder());
            wgPtr = folderPtr;
            connect(folderPtr, SIGNAL(pressed()), this, SLOT(onFolderPress()));
        }
        else if (itemPtr->isFile())
        {
            FileWiget* fileWgPtr = new FileWiget(itemPtr->toFile());
            wgPtr = fileWgPtr;
            fileWgPtr->installEventFilter(this);

            connect(fileWgPtr,  SIGNAL(modeChenged(bool)),
                    this,       SLOT(onFileWidgetModChanged(bool)));

            connect(fileWgPtr,  SIGNAL(tagClicked(QString)),
                    this,       SIGNAL(tagClicked(QString)));

        }

        wgPtr->setEdittingEnabled(getEdittingEnabled());
        wgPtr->setPrintEnabled(getPrintEnabled());

        connect(wgPtr,  SIGNAL(printRequested(File*)),
                this,   SIGNAL(printRequested(File*)));

        layoutPtr->addWidget(wgPtr);
        _widgets.append(wgPtr);
    }

    if (_currFolderPtr->getChildCount() == 0)
    {
        QLabel *labelPtr = new QLabel("Нет файлов");
        labelPtr->setAlignment(Qt::AlignHCenter);
        layoutPtr->addWidget(labelPtr);

    }

    // generate path
//    if (getRootFolderVisible() || _currFolderPtr != _rootFolderPtr)
//    {
    QLabel* labelPtr = new QLabel();
    labelPtr->setText(_currFolderPtr->getName());
    labelPtr->setIndent(5);
    ui->pathLayout->insertWidget(0, labelPtr);
//    }

    FolderItem* folderPtr2 = _currFolderPtr->getParent();

    for (; folderPtr2 != NULL; folderPtr2 = folderPtr2->getParent())
    {
        QToolButton* buttonPtr = new QToolButton();
        buttonPtr->setText(folderPtr2->getName());

        connect(buttonPtr,  SIGNAL(clicked()),
                this,       SLOT(onPathPressed()));

        ui->pathLayout->insertWidget(0, buttonPtr);

//        if (folderPtr2 == _currFolderPtr)
//            buttonPtr->setEnabled(false);
    }

    connect(_currFolderPtr, SIGNAL(structureChanged()),
            this,           SLOT(onFolderStructureChanged()));

    setMouseTrackingRecursive(this, true);

    while (QApplication::overrideCursor() != NULL)
        QApplication::restoreOverrideCursor();

    emit currFolderChanged();
}


bool FileBrowserWidget::getEdittingEnabled() const
{
    return _enableEdit;
}

bool FileBrowserWidget::getPrintEnabled() const
{
    return _enablePrint;
}

void FileBrowserWidget::setEdittingEnabled(bool enabled)
{
    _enableEdit = enabled;
    ui->topPanel->setHidden(_currFolderPtr == _rootFolderPtr && !getEdittingEnabled());

    foreach (FileTreeWidget *wgPtr, _widgets)
        wgPtr->setEdittingEnabled(enabled);

}

void FileBrowserWidget::setPrintEnabled(bool enabled)
{
    _enablePrint = enabled;

    foreach (FileTreeWidget *wgPtr, _widgets)
        wgPtr->setPrintEnabled(enabled);
}

bool FileBrowserWidget::eventFilter(QObject *, QEvent *eventPtr)
{
    if (eventPtr->type() == QEvent::MouseMove)
        mouseMoveEvent(static_cast<QMouseEvent *>(eventPtr));

    return false;
}


void FileBrowserWidget::onFolderPress()
{
    FolderWidget* folderPtr = dynamic_cast<FolderWidget*>(sender());

    if (folderPtr == NULL) return;

    setCurrFolder(folderPtr->getFolderPtr());
}


void FileBrowserWidget::onFileWidgetModChanged(bool expanded)
{
    FileWiget* newFileWgPtr = dynamic_cast<FileWiget*>(sender());

    if (expanded)
    {
        if (_currFileWgPtr != NULL && _currFileWgPtr != newFileWgPtr)
            _currFileWgPtr->changeMode(false);

        setCurrFileWidget(newFileWgPtr);
    }
    else
        setCurrFileWidget(NULL);

    ui->spacerWidget->setHidden(_currFileWgPtr != NULL
                                && _currFileWgPtr->isExpanded());
}

void FileBrowserWidget::onPathPressed()
{
    QToolButton* senderPtr = dynamic_cast<QToolButton*>(sender());

    if (senderPtr == NULL) return;

    int parNum = ui->pathLayout->count() - 1 - ui->pathLayout->indexOf(senderPtr);
    FolderItem *newFolderPtr = _currFolderPtr;

    for (int i = 0; i < parNum; ++i)
        newFolderPtr = newFolderPtr->getParent();

    setCurrFolder(newFolderPtr);
}

void FileBrowserWidget::onFolderStructureChanged()
{
    setCurrFolder(getCurrFolder());
}

void FileBrowserWidget::leaveEvent(QEvent *)
{
    _mouseHit = false;
    updateControlsVisible();
}

void FileBrowserWidget::mouseMoveEvent(QMouseEvent *)
{
    _mouseHit = ui->topPanel->rect()
            .contains(ui->topPanel->mapFromGlobal(QCursor::pos()));
    updateControlsVisible();
}

void FileBrowserWidget::updateControlsVisible()
{
    ui->toolPanel->setHidden(!_mouseHit || !getEdittingEnabled());
}

void FileBrowserWidget::on_addFileButton_clicked()
{
    FileDialog *dialogPtr = new FileDialog(getRootFolder(), this);

    if (dialogPtr->exec() != QDialog::Accepted)
        return;

    getCurrFolder()->addChild(new FileItem(dialogPtr->getFilePtr(), this));
}

void FileBrowserWidget::on_addFolderButton_clicked()
{
    getCurrFolder()->addChild(new FolderItem("Новая папка", this));

    /*
    FileTreeWidget *newWg = _widgets[getCurrFolder()->getChildFolderCount() - 1];
    FolderWidget *newFolderWg = dynamic_cast<FolderWidget *>(newWg);

    if (newFolderWg != NULL)
        newFolderWg->toggleNameMode(true);
    */
}

void FileBrowserWidget::on_addNewFileButton_clicked()
{
    QFileDialog dialog(this, "Выберите файл", "", tr("Image Files (*.png *.jpg *.bmp)"));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList fileNames;

    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    foreach (QString fileName, fileNames)
        getCurrFolder()->addChild(new FileItem(new File(fileName)));
}




// //// FileWiget

FileWiget::FileWiget(FileItem *fileItemPtr, QWidget *parent)
    : FileTreeWidget(fileItemPtr, parent), ui(new Ui::FileWidget), _isExpanded(false)
{
    ui->setupUi(this);
    ui->nameLabel->setText(getFileItemPtr()->getName());
    ui->bottomGroup->setHidden(true);
    ui->line->setHidden(true);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->scrollArea->setBackgroundRole(QPalette::Dark);

    onFileTagsChanged();

    connect(ui->tagLineEdit,    SIGNAL(editingFinished()),
            this,               SLOT(onTagEditingFinished()));

    connect(getFileItemPtr(),   SIGNAL(fileTagsChanged()),
            this,               SLOT(onFileTagsChanged()));

    connect(getFilePtr(),   SIGNAL(imageChanged()),
            this,           SLOT(onImageChanged()));

    toggleTagsMode(false);
    updateControlsVisible();

    setMouseTrackingRecursive(this, true);

    _headGroupHeight = ui->headGroup->height();

    if (getFilePtr()->getImage() != nullptr)
        onImageChanged();
    else
        ui->miniatureLabel->setFrameShape(QFrame::Box);
}

FileItem *FileWiget::getFileItemPtr() { return getItemPtr()->toFile(); }

File *FileWiget::getFilePtr() { return getFileItemPtr()->getFilePtr(); }

QScrollArea *FileWiget::getScrollArea()
{
    return ui->scrollArea;
}

bool FileWiget::isExpanded() const { return _isExpanded; }

void FileWiget::updateMouseHit()
{
    setMouseHit(ui->headGroup->rect()
                .contains(ui->headGroup->mapFromGlobal(QCursor::pos())));
}

void FileWiget::changeMode(bool expanded)
{
    if (_isExpanded == expanded)
        return;

    ui->bottomGroup->setHidden(!expanded);
    ui->miniatureLabel->setHidden(expanded);

    _isExpanded = expanded;
    emit modeChenged(expanded);
}


void FileWiget::updateControlsVisible()
{
    ui->editButton->setHidden(!getMouseHit() || !getEdittingEnabled());
    ui->deleteButton->setHidden(!getMouseHit() || !getEdittingEnabled());
    ui->printButton->setHidden(!getMouseHit() || !getPrintEnabled());

    ui->printButton->setEnabled(getFilePtr()->getImage() != nullptr);
}


void FileWiget::toggleTagsMode(bool enableEdit)
{
    ui->tagsPanel->setHidden(enableEdit);
    ui->tagEditPanel->setHidden(!enableEdit);
    ui->spacerWidget->setHidden(enableEdit);

    if (enableEdit)
    {
        ui->tagLineEdit->setText(getFileItemPtr()->getFilePtr()->getTagString());
        ui->tagLineEdit->setFocus();
    }
}


void FileWiget::onFileTagsChanged()
{
    clearLayout(ui->tagsPanel->layout());

    foreach (QString tag, getFileItemPtr()->getFilePtr()->getTags())
    {
        QToolButton* tagButtonPtr = new QToolButton();
        tagButtonPtr->setText(tag);
        ui->tagsPanel->layout()->addWidget(tagButtonPtr);

        connect(tagButtonPtr,   SIGNAL(clicked()),
                this,           SLOT(onTagClicked()));
    }
}

/*
void FileWiget::onSelectionStateChenged(FileTreeItem::SelectionState state)
{
    ui->printCheckBox->setCheckState((Qt::CheckState)state);
    updateControlsVisible();

//    qDebug() << "FileWiget::onSelectionStateChenged";
}
*/

void FileWiget::onTagEditingFinished()
{
    toggleTagsMode(false);
    getFileItemPtr()->getFilePtr()->inputTagsFromString(ui->tagLineEdit->text());
}

void FileWiget::onTagClicked()
{
    QToolButton *tagButtonPtr = dynamic_cast<QToolButton *>(sender());

    if (tagButtonPtr == NULL)
        return;

    emit tagClicked(tagButtonPtr->text());
}


void FileWiget::mousePressEvent(QMouseEvent *)
{
    if (getMouseHit())
        changeMode(!isExpanded());
}


void FileWiget::mouseMoveEvent(QMouseEvent *)
{
    makeMouseReaction();
}

/*
void FileWiget::on_printCheckBox_clicked()
{
    getFileItemPtr()->getFilePtr()->setSelectedToPrint(ui->printCheckBox->isChecked());
}
*/

void FileWiget::on_editButton_clicked()
{
    toggleTagsMode(true);
}

void FileWiget::on_deleteButton_clicked()
{
    QMessageBox *dialogPtr
            = new QMessageBox(QMessageBox::NoIcon,
                              "Удалить файл",
                              QString("Вы уверены, что хотите удалить файл %1?")
                              .arg(getFilePtr()->getName()),
                              QMessageBox::Yes | QMessageBox::No, this);

    dialogPtr->setIconPixmap(*ui->miniatureLabel->pixmap());
    dialogPtr->setButtonText(QMessageBox::Yes, "Да");
    dialogPtr->setButtonText(QMessageBox::No, "Нет");

    while (QApplication::overrideCursor() != NULL)
        QApplication::restoreOverrideCursor();

    if (dialogPtr->exec() == QMessageBox::Yes)
        getFileItemPtr()->getParent()->removeChild(getFileItemPtr());
}

void FileWiget::on_printButton_clicked()
{
    if (getFilePtr()->getImage() != nullptr)
        emit printRequested(getFilePtr());
}

void FileWiget::onImageChanged()
{
    if (getFilePtr()->getImage() == nullptr)
        return;

    ui->scrollArea->setMinimumHeight(getFilePtr()->getImage()->height());
    ui->imageLabel->setPixmap(*getFilePtr()->getImage());
    ui->miniatureLabel->setPixmap(getFilePtr()->getImage()
                                  ->scaledToHeight(_headGroupHeight * 1.5,
                                                   Qt::SmoothTransformation));
    ui->miniatureLabel->setFrameShape(QFrame::NoFrame);
}



// //// FolderWiget

FolderWidget::FolderWidget(FolderItem *folderItemPtr, QWidget *parent)
    : FileTreeWidget(folderItemPtr, parent), ui(new Ui::FolderWidget)
{
    ui->setupUi(this);
    ui->nameLabel->setMargin(4);
//    ui->printCheckBox->setCheckState((Qt::CheckState)getFolderPtr()->getSelectionState());

    connect(getFolderPtr(), SIGNAL(nameChanged(QString)),
            this,           SLOT(onFolderNameChanged(QString)));

    updateName();
    updateControlsVisible();
    toggleNameMode(false);

    ui->printCheckBox->setHidden(true); // !!!
}

FolderItem *FolderWidget::getFolderPtr()
{
    return getItemPtr()->toFolder();
}

void FolderWidget::updateControlsVisible()
{
    ui->editButton->setHidden(!getMouseHit() || !getEdittingEnabled());
    ui->deleteButton->setHidden(!getMouseHit() || !getEdittingEnabled());

    ui->printButton->setHidden(!getMouseHit() || !getPrintEnabled());

//    ui->printCheckBox->setEnabled(getSelectionEnabled());

    /*
    ui->printCheckBox->setHidden((!getMouseHit()
                                    && ui->printCheckBox->checkState() == Qt::Unchecked)
                                 || !getSelectionEnabled());
    */
}

void FolderWidget::toggleNameMode(bool enableEdit)
{
    ui->nameLineEdit->setHidden(!enableEdit);
    ui->nameLabel->setHidden(enableEdit);
}

void FolderWidget::updateName()
{
    ui->nameLabel->setText(getFolderPtr()->getName());
}

void FolderWidget::onFolderNameChanged(QString)
{
    updateName();
}

/*
void FolderWidget::onSelectionStateChenged(FileTreeItem::SelectionState state)
{
    ui->printCheckBox->setCheckState((Qt::CheckState)state);
}
*/

void FolderWidget::mousePressEvent(QMouseEvent *)
{
    QApplication::restoreOverrideCursor();
    emit pressed();
}


void FolderWidget::on_printCheckBox_clicked()
{
//    QCheckBox* checkBoxPtr = dynamic_cast<QCheckBox*>(sender());
//    getFolderPtr()->setSelectionRecursive(checkBoxPtr->isChecked());
}


void FolderWidget::on_deleteButton_clicked()
{
    QString dialogText;

    if (getFolderPtr()->getChildCount() == 0)
        dialogText = QString("Вы уверены, что хотите удалить папку %1?")
                .arg(getFolderPtr()->getName());
    else
        dialogText = QString("Папка %1 не пуста. Вы уверены, "
                             "что хотите удалить её вместе со всем содержимым?")
                .arg(getFolderPtr()->getName());

    QMessageBox *dialogPtr
            = new QMessageBox(QMessageBox::NoIcon,
                              "Удалить папку",
                              dialogText,
                              QMessageBox::Yes | QMessageBox::No, this);

    dialogPtr->setButtonText(QMessageBox::Yes, "Да");
    dialogPtr->setButtonText(QMessageBox::No, "Нет");

    while (QApplication::overrideCursor() != NULL)
        QApplication::restoreOverrideCursor();

    if (dialogPtr->exec() == QMessageBox::Yes)
    {
        if (getFolderPtr()->getParent() != NULL)
            getFolderPtr()->getParent()->removeChild(getFolderPtr());
    }
}

void FolderWidget::on_editButton_clicked()
{
    toggleNameMode(true);
    ui->nameLineEdit->setText(getFolderPtr()->getName());
    ui->nameLineEdit->setFocus();
}

void FolderWidget::on_nameLineEdit_editingFinished()
{
    getFolderPtr()->setName(ui->nameLineEdit->text());
    toggleNameMode(false);
}

void FolderWidget::requestPrintRecursive(FolderItem* folderPtr)
{
    for (int i = 0; i < folderPtr->getChildCount(); ++i)
        if (folderPtr->getChild(i)->isFile())
            emit printRequested(folderPtr->getChild(i)->toFile()->getFilePtr());
        else if (folderPtr->getChild(i)->isFolder())
            requestPrintRecursive(folderPtr->getChild(i)->toFolder());
}

void FolderWidget::on_printButton_clicked()
{
    requestPrintRecursive(getFolderPtr());
}



// //// FileTreeWidget

FileTreeWidget::FileTreeWidget(FileTreeItem *itemPtr, QWidget *parent)
    : QWidget(parent), _itemPtr(itemPtr)
    , _mouseHit(false), _enableEdit(true), _enablePrint(true)
{
    if (itemPtr == NULL)
        throw QString("try to create FileTreeWidget with invalid itemPtr");

    /*
    connect(_itemPtr,   SIGNAL(selectionStateCnahged(FileTreeItem::SelectionState)),
            this,       SLOT(onSelectionStateChenged(FileTreeItem::SelectionState)));
    */
}

FileTreeItem *FileTreeWidget::getItemPtr()
{
    return _itemPtr;
}

const FileTreeItem *FileTreeWidget::getItemPtr() const
{
    return _itemPtr;
}

bool FileTreeWidget::getMouseHit() const
{
    return _mouseHit;
}

bool FileTreeWidget::getEdittingEnabled() const
{
    return _enableEdit;
}

bool FileTreeWidget::getPrintEnabled() const
{
    return _enablePrint;
}

void FileTreeWidget::setEdittingEnabled(bool enabled)
{
    _enableEdit = enabled;
}

void FileTreeWidget::setPrintEnabled(bool enabled)
{
    _enablePrint = enabled;
}

void FileTreeWidget::updateMouseHit()
{
    _mouseHit = rect().contains(mapFromGlobal(QCursor::pos()));
}

void FileTreeWidget::makeMouseReaction()
{
    bool lastMouseHit = _mouseHit;
    updateMouseHit();

    if (!lastMouseHit && _mouseHit)
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
    else if (lastMouseHit && !_mouseHit)
        QApplication::restoreOverrideCursor();

    updateControlsVisible();
}

void FileTreeWidget::updateControlsVisible()
{
}

/*
void FileTreeWidget::onSelectionStateChenged(FileTreeItem::SelectionState state)
{
}
*/

void FileTreeWidget::leaveEvent(QEvent *)
{
    makeMouseReaction();
}

void FileTreeWidget::enterEvent(QEvent *)
{
    makeMouseReaction();
}

void FileTreeWidget::setMouseHit(bool mouseHit)
{
    _mouseHit = mouseHit;
}
