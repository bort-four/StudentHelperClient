#ifndef PRINTCOMPOSITOR_H
#define PRINTCOMPOSITOR_H

#include <QPixmap>
#include <QPrinter>
#include <QList>


class PrintCompositor
{
public:
    PrintCompositor(QPrinter &printer);

    void addPixmap(const QPixmap &pixmap);
    void composite();

    struct PartParams
    {
        int _partNum;
        QRect _rect;
    };

private:
    bool calculatePositions();
    void sortParams();
    void scaleMaximum();

    QList<QPixmap> _parts;
    QList<PartParams> _partParams;
    QPrinter &_printer;
};

#endif // PRINTCOMPOSITOR_H
