#include "treevalue.h"

#include <QPainter>

#include "component/enumclass.h"

TreeValue::TreeValue(const int* decimal, CStringHash* unit_symbol_hash, QObject* parent)
    : QStyledItemDelegate { parent }
    , decimal_ { decimal }
    , unit_symbol_hash_ { unit_symbol_hash }
{
    locale_ = QLocale(QLocale::Language::English, QLocale::Country::UnitedStates);
}

void TreeValue::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto string = locale_.toString(index.data().toDouble(), 'f', *decimal_);

    int unit { index.sibling(index.row(), static_cast<int>(TreeColumn::kUnit)).data().toInt() };
    string.prepend(unit_symbol_hash_->value(unit, ""));

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    } else
        painter->setPen(option.palette.color(QPalette::Text));

    painter->drawText(option.rect.adjusted(2, 0, -2, 0), Qt::AlignRight | Qt::AlignVCenter, string);
}

QSize TreeValue::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto string = locale_.toString(index.data().toDouble(), 'f', *decimal_);

    int unit { index.sibling(index.row(), static_cast<int>(TreeColumn::kUnit)).data().toInt() };
    string.prepend(unit_symbol_hash_->value(unit, ""));

    QFontMetrics fontMetrics(option.font);
    int width { fontMetrics.size(0, string).width() + 8 };

    return QSize(width, option.rect.height());
}
