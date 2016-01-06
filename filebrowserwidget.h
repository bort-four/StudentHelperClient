#ifndef FILEBROWSERWIDGET_H
#define FILEBROWSERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGridLayout>
#include <QScrollArea>
#include "../StudentHelperServer/filetreeitem.h"


namespace Ui {
class FileBrowserWidget;
class FileWidget;
class FolderWidget;
}


class FileWiget;
class FileTreeWidget;

class FileBrowserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileBrowserWidget(FolderItem* rootFolderPtr = NULL, QWidget *parent = 0);
    ~FileBrowserWidget();

    FolderItem *getRootFolder();
    FolderItem *getCurrFolder();

    FileWiget *getCurrFileWidget();
    void setCurrFileWidget(FileWiget *wgPtr);

    bool getRootFolderVisible() const;
    void setRootFolderVisible(bool visible);

    void setRootFolder(FolderItem* folderPtr);
    void setCurrFolder(FolderItem* folderPtr);

    bool getEdittingEnabled() const;
    bool getPrintEnabled() const;

    void setEdittingEnabled(bool enabled);
    void setPrintEnabled(bool enabled);

    virtual bool eventFilter(QObject *, QEvent *);

public slots:
    void onFolderPress();
    void onFileWidgetModChanged(bool expanded);
    void onPathPressed();
    void onFolderStructureChanged();

protected:
    virtual void leaveEvent(QEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

private slots:
    void on_addFileButton_clicked();

    void on_addFolderButton_clicked();

    void on_addNewFileButton_clicked();

signals:
    void currFileWidgetChanged(FileWiget *);
    void printRequested(File *);
    void tagClicked(QString);
    void currFolderChanged();

private:
    void updateControlsVisible();

    Ui::FileBrowserWidget *ui;
    FolderItem *_rootFolderPtr,
               *_currFolderPtr;
    FileWiget *_currFileWgPtr;
    QList<FileTreeWidget*> _widgets;
    bool _rootFolderVisible, _enableEdit, _enablePrint, _mouseHit;
};



class FileTreeWidget : public QWidget
{
    Q_OBJECT

public:
    FileTreeItem* getItemPtr();
    const FileTreeItem* getItemPtr() const;

    bool getEdittingEnabled() const;
    bool getPrintEnabled() const;

    void setEdittingEnabled(bool enabled);
    void setPrintEnabled(bool enabled);

    virtual void updateMouseHit();
    virtual void makeMouseReaction();
    virtual void updateControlsVisible();

signals:
    void printRequested(File*);

//public slots:
//    virtual void onSelectionStateChenged(FileTreeItem::SelectionState state);

protected:
    explicit FileTreeWidget(FileTreeItem* itemPtr, QWidget *parent = 0);

    virtual void leaveEvent(QEvent *);
    virtual void enterEvent(QEvent *);

    void setMouseHit(bool mouseHit);
    bool getMouseHit() const;

private:
    FileTreeItem* _itemPtr;
    bool _mouseHit, _enableEdit, _enablePrint;
};



class FileWiget : public FileTreeWidget
{
    Q_OBJECT

public:
    explicit FileWiget(FileItem* fileItemPtr, QWidget *parent = 0);

    FileItem* getFileItemPtr();
    File* getFilePtr();
    QScrollArea *getScrollArea();
    bool isExpanded() const;

    virtual void updateMouseHit();

    void changeMode(bool expanded);
    void toggleTagsMode(bool enableEdit);

public slots:
    void updateControlsVisible();
    void onFileTagsChanged();
    void onTagEditingFinished();
    void onTagClicked();
//    virtual void onSelectionStateChenged(FileTreeItem::SelectionState state);

signals:
    void modeChenged(bool);
    void tagClicked(QString);

protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

private slots:
    void on_editButton_clicked();
//    void on_printCheckBox_clicked();
    void on_deleteButton_clicked();
    void on_printButton_clicked();
    void onImageChanged();

private:
    Ui::FileWidget *ui;
    bool _isExpanded;
    int _headGroupHeight = 0;
};



class FolderWidget : public FileTreeWidget
{
    Q_OBJECT

public:
    explicit FolderWidget(FolderItem* folderItemPtr, QWidget *parent = 0);
    FolderItem* getFolderPtr();

    virtual void updateControlsVisible();
    void toggleNameMode(bool enableEdit);

public slots:
    void updateName();
    void onFolderNameChanged(QString);
//    virtual void onSelectionStateChenged(FileTreeItem::SelectionState state);

signals:
    void pressed();

protected:
    virtual void mousePressEvent(QMouseEvent *);

private slots:
    void on_printCheckBox_clicked();
    void on_deleteButton_clicked();
    void on_editButton_clicked();
    void on_nameLineEdit_editingFinished();
    void on_printButton_clicked();

private:
    void requestPrintRecursive(FolderItem *folderPtr);

    Ui::FolderWidget *ui;
};




#endif // FILEBROWSERWIDGET_H
