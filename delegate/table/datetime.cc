#include "datetime.h"

#include <QPainter>

#include "widget/datetimeedit.h"

DateTime::DateTime(const QString* date_format, const bool* hide_time, QObject* parent)
    : QStyledItemDelegate { parent }
    , date_format_ { date_format }
    , hide_time_ { hide_time }
{
    time_pattern_ = QRegularExpression(R"((\s?\S{2}:\S{2}\s?))");
}

QWidget* DateTime::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new DateTimeEdit(parent) };
    editor->setButtonSymbols(QAbstractSpinBox::NoButtons);

    editor->setDisplayFormat(*date_format_);
    return editor;
}

void DateTime::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<DateTimeEdit*>(editor) };
    last_insert_ = last_insert_.isValid() ? last_insert_ : QDateTime::currentDateTime();

    auto date_time { index.data().toDateTime() };
    if (!date_time.isValid()) {
        date_time = last_insert_;
        auto time { QTime::currentTime() };
        date_time.setTime(time);
    }

    cast_editor->setDateTime(date_time);
}

void DateTime::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void DateTime::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<DateTimeEdit*>(editor) };
    last_insert_ = cast_editor->dateTime();
    model->setData(index, cast_editor->text());
}

void DateTime::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto string { index.data().toString() };

    if (string.isEmpty())
        return QStyledItemDelegate::paint(painter, option, index);

    if (*hide_time_)
        string.remove(time_pattern_);

    if (option.state & QStyle::State_Selected)
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    else
        painter->setPen(option.palette.color(QPalette::Text));

    painter->drawText(option.rect.adjusted(2, 0, -2, 0), Qt::AlignCenter, string);
}

QSize DateTime::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto string { index.data().toString() };

    if (string.isEmpty())
        return QSize();

    if (*hide_time_)
        string.remove(time_pattern_);

    QFontMetrics fontMetrics(option.font);
    int width { fontMetrics.size(0, string).width() + 4 };

    return QSize(width, option.rect.height());
}
