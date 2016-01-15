#include "printerwidget.h"
#include "ui_printerwidget.h"
#include <QPainter>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QPrintPreviewDialog>
#include "printcompositor.h"

PrinterWidget::PrinterWidget(QWidget *parent, StudentHelperContent *h_data,
                             SHCache *cache) :
    QWidget(parent),
    ui(new Ui::PrinterWidget),
    helper_data(h_data),
    system_cache(cache)
{
    ui->setupUi(this);

    connect(helper_data, SIGNAL(sendToPrint(File*)), this, SLOT(addToQueue(File*)) );

    connect(ui->print_list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(showInfo(QListWidgetItem*)));
    connect(ui->corner_coord_x, SIGNAL(editingFinished()), this, SLOT(newSize()));
    connect(ui->corner_coord_y, SIGNAL(editingFinished()), this, SLOT(newSize()));
    connect(ui->new_size_x, SIGNAL(editingFinished()), this, SLOT(newSize()));
    connect(ui->new_size_y, SIGNAL(editingFinished()), this, SLOT(newSize()));
    connect(ui->cut_button, SIGNAL(clicked(bool)), this, SLOT(cut()));
    connect(ui->cancel_button, SIGNAL(clicked(bool)), this, SLOT(undo()));
    connect(ui->rotate_left_button, SIGNAL(clicked(bool)), this, SLOT(rotateLeft()));
    connect(ui->rotate_right_button, SIGNAL(clicked(bool)), this, SLOT(rotateRight()));
    connect(ui->discolour_button, SIGNAL(clicked(bool)), this, SLOT(discolor()));
    connect(ui->delete_selected_button, SIGNAL(clicked(bool)), this, SLOT(deleteSelectedItems()));
    connect(ui->print_button, SIGNAL(clicked(bool)), this, SLOT(printOneByOne()));
    connect(ui->compose_button, SIGNAL(clicked(bool)), this, SLOT(composeAndPrint()));
    connect(ui->selection_button, SIGNAL(clicked(bool)), this, SLOT(selectAll()));
    connect(ui->scale_minus_button, SIGNAL(clicked(bool)), this, SLOT(scale()));
    connect(ui->scale_plus_button, SIGNAL(clicked(bool)), this, SLOT(scale()));

    edit_history = new QMap<QString,QList<QPixmap*> >;
    current_states = new QMap<QString,QPixmap*>;
    copies_table = new QMap<QString,int>;

    work_pix = NULL;
    cut_rect_show_pix = NULL;
    currentItem = NULL;
    previousItem = NULL;
}

PrinterWidget::~PrinterWidget()
{
    delete ui;
}

void PrinterWidget::addCurrentState(const QString& fname, const QPixmap& pix)
{
    current_states->insert(fname,new QPixmap(pix));
}

// заменяемое значение УДАЛЯЕТСЯ (delete)
void PrinterWidget::resetCurrentState(const QString& fname, const QPixmap& pix)
{
    QMap<QString,QPixmap*>::iterator it = current_states->find(fname);
    if (it != current_states->end())
    {
        QPixmap* vpix = it.value();
        if (vpix != NULL)
            delete vpix;
    }
    current_states->insert(fname, new QPixmap(pix));
}

void PrinterWidget::removeCurrentState(const QString& fname)
{
    QMap<QString,QPixmap*>::iterator it = current_states->find(fname);
    if (it == current_states->end())
    {
        return;
    }
    delete it.value();
    current_states->erase(it);
}

void PrinterWidget::addToQueue(File* fPtr)
{
    if (fPtr == NULL)
    {
        return;
    }

    const QString& name = fPtr->getName();
//    QString path = fPtr->getFullName();
    QString path = fPtr->getName();
    QListWidget& lst = *ui->print_list;

    int item_num = insertToTable(path);
    QString suffix = "(" + QString::number(item_num) + ")";
    path += suffix;
    QListWidgetItem* item = new QListWidgetItem( item_num == 1 ? name : name + suffix, &lst);
    item->setData(3,path);
    item->setFlags( item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);

    addCurrentState(path, *fPtr->getImage());
}

int PrinterWidget::insertToTable(const QString& key)
{
    QMap<QString,int>::iterator it = copies_table->find(key);
    if (it == copies_table->end())
    {
        copies_table->insert(key,1);
    }
    else
    {
        return ++it.value();
    }
    return 1;
}

void PrinterWidget::deleteQueueItem(QListWidgetItem* item)
{
    QListWidget& lst = *ui->print_list;
    const QString& path = item->data(3).toString();

    clearHistory(edit_history->find(path));

    if (previousItem == item)
    {
        previousItem = NULL;
    }
    if (currentItem == item)
    {
        currentItem = NULL;
        QLabel* l = dynamic_cast<QLabel*>(ui->monitor_area->widget());
        if (cut_rect_show_pix != NULL)
        {
            delete cut_rect_show_pix;
        }
        if (work_pix != NULL)
        {
            delete work_pix;
        }
        work_pix = new QPixmap;
        cut_rect_show_pix = new QPixmap;
        setRedBox(cut_rect_show_pix, cut_rect_show_pix->rect());
        l->setPixmap(*cut_rect_show_pix);
        resetCutParameters(0,0);
    }
    removeCurrentState(path);
    delete item;

    if (lst.count() == 0)
    {
        ui->selection_button->setText("Выделить всё");
        copies_table->clear();
    }
}

void PrinterWidget::clearHistory(QMap<QString,QList<QPixmap*> >::iterator it)
{
    if ( it == edit_history->end())
    {
        return;
    }
    QList<QPixmap*>& lst = it.value();
    for (QList<QPixmap*>::iterator itr = lst.begin(); itr != lst.end(); ++itr)
    {
        delete *itr;
    }
    edit_history->erase(it);
}

void PrinterWidget::resetCutParameters(int width, int height)
{
    ui->current_width->setText(QString::number(width));
    ui->current_heigh->setText(QString::number(height));

    ui->corner_coord_x->setValue(0);
    ui->corner_coord_y->setValue(0);
    ui->new_size_x->setValue(width);
    ui->new_size_y->setValue(height);
}

void PrinterWidget::showInfo(QListWidgetItem* item)
{
    previousItem = currentItem;
    currentItem = item;

    if ( currentItem == previousItem )
    {
        return;
    }

    QMap<QString,QPixmap*>::iterator it = current_states->find(item->data(3).toString());
    work_pix = (it != current_states->end()) ? new QPixmap(*it.value()) : new QPixmap;

    QLabel* l = dynamic_cast<QLabel*>(ui->monitor_area->widget());
    if (cut_rect_show_pix != NULL)
    {
        delete cut_rect_show_pix;
    }
    cut_rect_show_pix = new QPixmap(*work_pix);
    setRedBox(cut_rect_show_pix, cut_rect_show_pix->rect());
    if (l != NULL)
    {
        l->setPixmap(*cut_rect_show_pix);
    }
    else
    {
        QLabel* lbl = new QLabel;
        lbl->setPixmap(*cut_rect_show_pix);
        ui->monitor_area->setWidget(lbl);
    }    
    resetCutParameters(cut_rect_show_pix->width(), cut_rect_show_pix->height());
}

void PrinterWidget::newSize()
{
    if (ui->print_list->count() == 0)
    {
        return;
    }
    QLabel* l = dynamic_cast<QLabel*>(ui->monitor_area->widget());
    if (currentItem == NULL)
    {
        if (l == NULL)
        {
            return;
        }
        if (cut_rect_show_pix != NULL)
        {
            delete cut_rect_show_pix;
        }
        if (work_pix == NULL)
        {
            work_pix = new QPixmap;
            cut_rect_show_pix = new QPixmap;
        }
        else
        {
            cut_rect_show_pix = new QPixmap(*work_pix);
            setRedBox(cut_rect_show_pix, cut_rect_show_pix->rect());
        }
        l->setPixmap(*cut_rect_show_pix);
        return;
    }

    int new_x = ui->corner_coord_x->value();
    int new_y = ui->corner_coord_y->value();
    int new_w = ui->new_size_x->value();
    int new_h = ui->new_size_y->value();

    int pix_w = work_pix->width();
    int pix_h = work_pix->height();
    int x = pix_w - new_x <= 0 ? pix_w : new_x;
    int y = pix_h - new_y <= 0 ? pix_h : new_y;
    int w = new_w - (pix_w-x) >= 0 ? pix_w-x : new_w;
    int h = new_h - (pix_h-y) >= 0 ? pix_h-y : new_h;

    if (cut_rect_show_pix != NULL)
    {
        delete cut_rect_show_pix;
    }
    cut_rect_show_pix = new QPixmap(*work_pix);
    setRedBox(cut_rect_show_pix, QRect(x,y,w,h));
    l->setPixmap(*cut_rect_show_pix);
}

void PrinterWidget::setRedBox(QPixmap* pixmap, const QRect& rect)
{
    if (pixmap->size().isEmpty())
    {
        return;
    }
    QPainter p(pixmap);
    QPen pen(Qt::red, 1, Qt::DashLine);
    p.setPen(pen);
    p.drawRect(rect.left(), rect.top(), rect.width()-1, rect.height()-1);
}

void PrinterWidget::addToHistory(const QString& fname, const QPixmap& pix)
{
    QMap<QString,QList<QPixmap*> >::iterator it = edit_history->find(fname);
    QPixmap* p = new QPixmap(pix);
    if (it != edit_history->end())
    {
        it->push_back(p);
    }
    else
    {
        edit_history->insert(fname, (QList<QPixmap*>() << p) );
    }
}

void PrinterWidget::cut()
{
    if (ui->print_list->count() == 0)
    {
        return;
    }

    int x = ui->corner_coord_x->value();
    int y = ui->corner_coord_y->value();
    int w = ui->new_size_x->value();
    int h = ui->new_size_y->value();

    if (x == 0 && y == 0 && w == work_pix->width() && h == work_pix->height())
    {
        return;
    }

    const QString& path = currentItem->data(3).toString();
    QPixmap* p = new QPixmap(work_pix->copy(x,y,w,h));

    addToHistory(path, *work_pix);
    resetCurrentState(path, *p);
    delete work_pix;
    work_pix = p;
    if (cut_rect_show_pix != NULL)
    {
        delete cut_rect_show_pix;
    }
    cut_rect_show_pix = new QPixmap(*work_pix);
    setRedBox(cut_rect_show_pix, cut_rect_show_pix->rect());
    QLabel* l = dynamic_cast<QLabel*>(ui->monitor_area->widget());
    l->setPixmap(*cut_rect_show_pix);

    resetCutParameters(work_pix->width(), work_pix->height());
}

void PrinterWidget::undo()
{
    if (ui->print_list->count() == 0 || edit_history == NULL)
    {
        return;
    }
    const QString& path = currentItem->data(3).toString();
    QMap<QString,QList<QPixmap*> >::iterator it = edit_history->find(path);
    if (it == edit_history->end())
    {
        return;
    }
    if (it->empty())
    {
        return;
    }

    QPixmap* pix = it->back();
    it->pop_back();

    resetCurrentState(path,*pix);
    delete work_pix;
    work_pix = pix;

    QLabel* l = dynamic_cast<QLabel*>(ui->monitor_area->widget());
    if (cut_rect_show_pix != NULL)
    {
        delete cut_rect_show_pix;
    }
    cut_rect_show_pix = new QPixmap(*work_pix);
    setRedBox(cut_rect_show_pix, QRect(0,0,cut_rect_show_pix->width()-1,cut_rect_show_pix->height()-1));
    l->setPixmap(*cut_rect_show_pix);

    resetCutParameters(work_pix->width(), work_pix->height());
}

void PrinterWidget::rotateLeft()
{
    if (ui->print_list->count() == 0)
    {
        return;
    }

    const QString& path = currentItem->data(3).toString();

    addToHistory(path, *work_pix);

    QPixmap* pix = new QPixmap(work_pix->height(), work_pix->width());
    QPainter p(pix);
    p.translate(0,work_pix->width());
    p.rotate(-90);
    p.drawPixmap(work_pix->rect(), *work_pix);

    resetCurrentState(path,*pix);
    delete work_pix;
    work_pix = pix;
    if (cut_rect_show_pix != NULL)
    {
        delete cut_rect_show_pix;
    }
    cut_rect_show_pix = new QPixmap(*work_pix);
    setRedBox(cut_rect_show_pix, cut_rect_show_pix->rect());
    QLabel* l = dynamic_cast<QLabel*>(ui->monitor_area->widget());
    l->setPixmap(*cut_rect_show_pix);
    resetCutParameters(cut_rect_show_pix->width(), cut_rect_show_pix->height());
}

void PrinterWidget::rotateRight()
{
    if (ui->print_list->count() == 0)
    {
        return;
    }

    const QString& path = currentItem->data(3).toString();

    addToHistory(path, *work_pix);

    QPixmap* pix = new QPixmap(work_pix->height(), work_pix->width());
    QPainter p(pix);
    p.translate(work_pix->height(),0);
    p.rotate(90);
    p.drawPixmap(work_pix->rect(), *work_pix);

    resetCurrentState(path,*pix);
    delete work_pix;
    work_pix = pix;
    if (cut_rect_show_pix != NULL)
    {
        delete cut_rect_show_pix;
    }
    cut_rect_show_pix = new QPixmap(*work_pix);
    setRedBox(cut_rect_show_pix, cut_rect_show_pix->rect());
    QLabel* l = dynamic_cast<QLabel*>(ui->monitor_area->widget());
    l->setPixmap(*cut_rect_show_pix);
    resetCutParameters(cut_rect_show_pix->width(), cut_rect_show_pix->height());
}

void PrinterWidget::scale()
{
    if (ui->print_list->count() == 0)
    {
        return;
    }

    double sx, sy;
    sx = (sender() == ui->scale_plus_button) ? sy = 1.1 : sy = 0.9;

    const QString& path = currentItem->data(3).toString();

    addToHistory(path, *work_pix);

    QSize size(sx*work_pix->width(),sy*work_pix->height());
    QPixmap* pix = new QPixmap( work_pix->scaled(size,Qt::KeepAspectRatio) );

    resetCurrentState(path,*pix);
    delete work_pix;
    work_pix = pix;
    if (cut_rect_show_pix != NULL)
    {
        delete cut_rect_show_pix;
    }
    cut_rect_show_pix = new QPixmap(*work_pix);
    setRedBox(cut_rect_show_pix, cut_rect_show_pix->rect());
    QLabel* l = dynamic_cast<QLabel*>(ui->monitor_area->widget());
    l->setPixmap(*cut_rect_show_pix);
    resetCutParameters(cut_rect_show_pix->width(), cut_rect_show_pix->height());
}

void PrinterWidget::discolor()
{
    if (ui->print_list->count() == 0)
    {
        return;
    }

    const QString& path = currentItem->data(3).toString();

    addToHistory(path, *work_pix);

    QImage img = work_pix->toImage();
    for( int w = 0; w < img.width(); ++w )
    {
        for( int h = 0; h < img.height(); ++h )
        {
            QColor col( img.pixel(w,h) );
            col.setHsv(col.hue(), 0, col.value(), col.alpha());
            img.setPixel(w,h,col.rgb());
        }
    }
    QPixmap* pix = new QPixmap( QPixmap::fromImage(img) );
    resetCurrentState(path,*pix);
    delete work_pix;
    work_pix = pix;
    if (cut_rect_show_pix != NULL)
    {
        delete cut_rect_show_pix;
    }
    cut_rect_show_pix = new QPixmap(*work_pix);
    setRedBox(cut_rect_show_pix, cut_rect_show_pix->rect());
    QLabel* l = dynamic_cast<QLabel*>(ui->monitor_area->widget());
    l->setPixmap(*cut_rect_show_pix);
    resetCutParameters(cut_rect_show_pix->width(), cut_rect_show_pix->height());
}

void PrinterWidget::deleteSelectedItems()
{
    QListWidget& lst = *ui->print_list;
    if (lst.count() == 0)
    {
        return;
    }

    for(int i = lst.count()-1; i != -1 ; --i)
    {
        QListWidgetItem* item = lst.item(i);
        if (item->checkState() == Qt::Checked)
        {
            deleteQueueItem(item);
        }
    }
}

void PrinterWidget::selectAll()
{
    if (ui->print_list->count() == 0)
    {
        return;
    }

    Qt::CheckState status1, status2;
    QPushButton* pb = ui->selection_button;
    if (pb->text() == "Выделить всё")
    {
        pb->setText("Снять отметки");
        status1 = Qt::Unchecked;
        status2 = Qt::Checked;
    }
    else
    {
        pb->setText("Выделить всё");
        status1 = Qt::Checked;
        status2 = Qt::Unchecked;
    }

    QListWidget& lst = *ui->print_list;
    for(int i = 0; i < lst.count(); ++i)
    {
        QListWidgetItem* item = lst.item(i);
        if (item->checkState() == status1)
        {
            item->setCheckState(status2);
        }
    }
}

QList<QPixmap*>& PrinterWidget::getSelectedPixes()
{
    QList<QPixmap*>& pixes_to_print = *(new QList<QPixmap*>);
    const QListWidget& lst = *ui->print_list;
    for(int i = 0; i < lst.count(); ++i)
    {
        QListWidgetItem* item = lst.item(i);
        if (item->checkState() == Qt::Checked)
        {
            QMap<QString,QPixmap*>::iterator it = current_states->find(item->data(3).toString());
            if (it == current_states->end())
            {
                continue;
            }
            QPixmap* pix = it.value();
            if (pix == NULL)
            {
                continue;
            }
            pixes_to_print.append(pix);
        }
    }
    return pixes_to_print;
}

void PrinterWidget::composeAndPrint()
{
    if (ui->print_list->count() == 0)
    {
        return;
    }

    QPrinter printer;
    QPrintPreviewDialog preview( &printer, this );
    connect( &preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(print_composed(QPrinter*)) );
    preview.exec();

}

void PrinterWidget::print_composed(QPrinter* printer)
{
    QList<QPixmap*> pixes_to_print = getSelectedPixes();
    PrintCompositor composer(*printer);
    for (QList<QPixmap*>::iterator it = pixes_to_print.begin(); it != pixes_to_print.end(); ++it)
    {
        composer.addPixmap(**it);
    }
    composer.composite();
}

void PrinterWidget::printOneByOne()
{
    if (ui->print_list->count() == 0)
        return;

    QPrinter printer;
    QPrintPreviewDialog preview( &printer, this );
    connect( &preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(print_selected(QPrinter*)) );
    preview.exec();
}

void PrinterWidget::print_selected(QPrinter* printer)
{
    QList<QPixmap*> pixes_to_print = getSelectedPixes();
    QPainter painter;
    painter.begin(printer);
    for (QList<QPixmap*>::iterator it = pixes_to_print.begin(); it != pixes_to_print.end(); ++it)
    {
        if (it != pixes_to_print.begin())
        {
            printer->newPage();
        }
        painter.drawPixmap( (*it)->rect(), *(*it) );
    }
    painter.end();
}
