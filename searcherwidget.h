#ifndef SEARCHERWIDGET_H
#define SEARCHERWIDGET_H

#include <QWidget>
#include "../StudentHelperServer/studenthelpercontent.h"
#include <QListWidget>
#include "filebrowserwidget.h"

namespace Ui
{
    class SearcherWidget;
}

class SearcherWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearcherWidget(QWidget *parent = 0, StudentHelperContent *hlpr = 0);
    ~SearcherWidget();

private:
    void searchStart(const QList<File*>&);
    QString prepareQueryString();
    void emptyResults();

public slots:
    void localSearching();
    void baseSearching();
    void searchTypeSelected();
    void selectAll();
    void tagSearchInit(QString);

private:
    Ui::SearcherWidget* ui;
    StudentHelperContent*      helper_data;
    char                searching_type;
    QList<File*>*       temp_searching_results;
    FileBrowserWidget*  browser;
};

#endif // SEARCHERWIDGET_H
