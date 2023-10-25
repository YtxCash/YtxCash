#include "nodename.h"

#include <QPainter>

#include "component/enumclass.h"

NodeName::NodeName(CStringHash* leaf_path, CStringHash* branch_path, QObject* parent)
    : QStyledItemDelegate { parent }
    , leaf_path_ { leaf_path }
    , branch_path_ { branch_path }
{
}

void NodeName::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto path { GetPath(index) };
    if (path.isEmpty())
        return QStyledItemDelegate::paint(painter, option, index);

    painter->setPen(option.state & QStyle::State_Selected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text));
    painter->drawText(option.rect.adjusted(4, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, path);
}

QSize NodeName::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto path { GetPath(index) };
    return path.isEmpty() ? QSize() : QSize(QFontMetrics(option.font).horizontalAdvance(path) + 8, option.rect.height());
}

QString NodeName::GetPath(const QModelIndex& index) const
{
    int node_id { index.sibling(index.row(), static_cast<int>(NodeColumn::kID)).data().toInt() };

    auto hash { leaf_path_->contains(node_id) ? leaf_path_ : branch_path_ };
    return hash->value(node_id, QString());
}
