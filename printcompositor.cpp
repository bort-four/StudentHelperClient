#include "printcompositor.h"
#include <QPainter>
#include <QtCore>
#include <QtAlgorithms>


PrintCompositor::PrintCompositor(QPrinter &printer)
    : _printer(printer)
{
}

void PrintCompositor::composite()
{
    while (!calculatePositions())
        scaleMaximum();

    QPainter painter;

    if (!painter.begin(&_printer))
    {
        qDebug() << "Can't open printer(?)";
        return;
    }

//    foreach (auto part, _partParams)
//        qDebug() << part._partNum << part._rect;

    painter.drawRect(painter.viewport());

    for (int i = 0; i < _partParams.size(); ++i)
    {
        const PartParams &params = _partParams[i];

        if (params._rect == _parts[params._partNum].rect())
            painter.drawPixmap(params._rect.x(), params._rect.y(), _parts[params._partNum]);
        else
            painter.drawPixmap(params._rect.x(), params._rect.y(),
                               _parts[params._partNum]
                    .scaled(params._rect.width(), params._rect.height(),
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    painter.end();
}


bool PrintCompositor::calculatePositions()
{
    sortParams();

    int W = _printer.width();
    int H = _printer.height();

    int next_h = 0, h = 0, w = 0;

    for (int i = 0; i < _partParams.size(); ++i)
    {
        PartParams params = _partParams[i];

        if (W - w < params._rect.width())
        {
            h = next_h;
            w = 0;
        }

        if (w == 0)
            next_h += params._rect.height();

        if (W - w < params._rect.width() || h + params._rect.height() > H)
            return false;

        params._rect.moveTo(QPoint(w, h));
        w += params._rect.width();

        _partParams[i] = params;
   }

    return true;
}

bool paramsCmp(PrintCompositor::PartParams &param1,
               PrintCompositor::PartParams &param2)
{
    return param1._rect.height() > param2._rect.height();
}

void PrintCompositor::sortParams()
{
    qSort(_partParams.begin(), _partParams.end(), paramsCmp);
}

void PrintCompositor::scaleMaximum()
{
    int maxPartNum = 0;
    int maxPartArea = -1;

    for (int i = 0; i < _partParams.count(); ++i)
        if (_partParams[i]._rect.height() * _partParams[i]._rect.width() > maxPartArea)
        {
            maxPartNum = i;
            maxPartArea = _partParams[i]._rect.height() * _partParams[i]._rect.width();
        }

    const double K = 0.98;

    PartParams params = _partParams[maxPartNum];

    params._rect.setWidth((int)(_partParams[maxPartNum]._rect.width()*K));
    params._rect.setHeight((int)(_partParams[maxPartNum]._rect.height()*K));

    _partParams[maxPartNum] = params;
}


void PrintCompositor::addPixmap(const QPixmap &pixmap)
{
    _parts.append(pixmap);
    _partParams.append({_parts.count() - 1, pixmap.rect()});
}




