#include "tablevalue.h"

#include <QPainter>

#include "component/enumclass.h"
#include "widget/doublespinbox.h"

TableValue::TableValue(const int* decimal, QObject* parent)
    : QStyledItemDelegate { parent }
    , decimal_ { decimal }
{
    locale_ = QLocale(QLocale::Language::English, QLocale::Country::UnitedStates);
}

QWidget* TableValue::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new DoubleSpinBox(parent) };
    editor->setDecimals(*decimal_);
    editor->setRange(-1000000000, 1000000000);
    editor->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    editor->setButtonSymbols(QAbstractSpinBox::NoButtons);
    editor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    editor->setGroupSeparatorShown(true);

    return editor;
}

void TableValue::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<DoubleSpinBox*>(editor) };
    cast_editor->setValue(index.data().toDouble());
}

void TableValue::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void TableValue::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<DoubleSpinBox*>(editor) };
    model->setData(index, cast_editor->value());
}

void TableValue::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toDouble() };

    if (value == 0 && index.column() != static_cast<int>(TableColumn::kBalance))
        return QStyledItemDelegate::paint(painter, option, index);

    if (option.state & QStyle::State_Selected)
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    else
        painter->setPen(option.palette.color(QPalette::Text));

    auto string { locale_.toString(value, 'f', *decimal_) };

    painter->drawText(option.rect.adjusted(2, 0, -2, 0), Qt::AlignRight | Qt::AlignVCenter, string);
}

QSize TableValue::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toDouble() };
    if (value == 0)
        return QSize();

    auto string { locale_.toString(value, 'f', *decimal_) };
    QFontMetrics fontMetrics(option.font);
    int width { fontMetrics.size(0, string).width() + 8 };

    return QSize(width, option.rect.height());
}
