#include "doublespinbox.h"

#include <QKeyEvent>

DoubleSpinBox::DoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox { parent }
{
}

void DoubleSpinBox::keyPressEvent(QKeyEvent* event)
{
    auto key { event->key() };

    switch (key) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        this->setValue(0);
        break;
    default:
        return QDoubleSpinBox::keyPressEvent(event);
    }
}

void DoubleSpinBox::wheelEvent(QWheelEvent* event) { event->ignore(); }
