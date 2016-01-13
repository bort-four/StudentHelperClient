#ifndef PRINTERWIDGET_H
#define PRINTERWIDGET_H

#include <QWidget>
#include "../StudentHelperServer/studenthelpercontent.h"
#include "../StudentHelperServer/shquery.h"
#include <QListWidget>
#include <QPrinter>
#include "cachingbase.h"

namespace Ui
{
    class PrinterWidget;
}

class PrinterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrinterWidget(QWidget *parent = 0, StudentHelperContent* = 0,
                           SHCache* cache = 0);
    ~PrinterWidget();

private:
    void addToHistory(const QString&fname, const QPixmap &pix);
    void resetCutParameters(int width, int height);
    void setRedBox(QPixmap *pixmap, const QRect &rect);
    void addCurrentState(const QString&fname, const QPixmap &pix);
    void resetCurrentState(const QString&fname, const QPixmap &pix);
    void removeCurrentState(const QString&fname);
    QList<QPixmap *> &getSelectedPixes();
    void clearHistory(QMap<QString,QList<QPixmap*> >::iterator it);
    void deleteQueueItem(QListWidgetItem *item);
    int  insertToTable(const QString& key);

public slots:
    void addToQueue(File* fPtr);

private slots:
    void showInfo(QListWidgetItem*);
    void newSize();
    void cut();
    void undo();
    void rotateLeft();
    void rotateRight();
    void discolor();
    void deleteSelectedItems();
    void selectAll();
    void printOneByOne();
    void composeAndPrint();
    void print_selected(QPrinter* printer);
    void print_composed(QPrinter* printer);
    void scale();

private:
    Ui::PrinterWidget *ui;
    StudentHelperContent* helper_data;
    SHCache* system_cache;

    QPixmap* work_pix;
    QPixmap* cut_rect_show_pix;
    QListWidgetItem* currentItem;
    QListWidgetItem* previousItem;

    QMap<QString,QList<QPixmap*> >* edit_history;
    QMap<QString,QPixmap*>* current_states;
    QMap<QString,int>* copies_table;
};

#endif // PRINTERWIDGET_H
