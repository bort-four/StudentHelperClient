#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QDialog>
#include "filebrowserwidget.h"

namespace Ui {
class FileDialog;
}

class FileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileDialog(FolderItem *rootFolderPtr, QWidget *parent = 0);
    ~FileDialog();

    File *getFilePtr();

private slots:
    void onCurrFileWidgetChanged(FileWiget *);

    void on_cancelButton_clicked();

    void on_chooseButton_clicked();

private:
    Ui::FileDialog *ui;
    FileBrowserWidget *_browserPtr;
};

#endif // FILEDIALOG_H
