#include "datetimeedit.h"

#include <QKeyEvent>

DateTimeEdit::DateTimeEdit(QWidget* parent)
    : QDateTimeEdit { parent }
{
}

void DateTimeEdit::keyPressEvent(QKeyEvent* event)
{
    auto date_time { this->dateTime() };
    auto key { event->key() };

    switch (key) {
    case Qt::Key_H:
        date_time = date_time.addDays(-1);
        break;
    case Qt::Key_L:
        date_time = date_time.addDays(1);
        break;
    case Qt::Key_J:
        date_time = date_time.addDays(-7);
        break;
    case Qt::Key_K:
        date_time = date_time.addDays(7);
        break;
    case Qt::Key_W:
        LastMonthEnd(date_time);
        break;
    case Qt::Key_B:
        NextMonthStart(date_time);
        break;
    case Qt::Key_N:
        date_time = date_time.addYears(1);
        break;
    case Qt::Key_E:
        date_time = date_time.addYears(-1);
        break;
    case Qt::Key_T:
        date_time = QDateTime::currentDateTime();
        break;
    default:
        return QDateTimeEdit::keyPressEvent(event);
    }

    if (date_time.isValid()) {
        this->setDateTime(date_time);
    }
}

void DateTimeEdit::LastMonthEnd(QDateTime& date_time)
{
    auto date { date_time.addMonths(-1).date() };
    date = QDate(date.year(), date.month(), date.daysInMonth());
    date_time.setDate(date);
}

void DateTimeEdit::NextMonthStart(QDateTime& date_time)
{
    auto date = date_time.addMonths(1).date();
    date = QDate(date.year(), date.month(), 1);
    date_time.setDate(date);
}
