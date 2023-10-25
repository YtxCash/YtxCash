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
    int node_id { index.sibling(index.row(), static_cast<int>(TreeColumn::kID)).data().toInt() };

    const CStringHash* hash_path {};
    leaf_path_->contains(node_id) ? hash_path = leaf_path_ : hash_path = branch_path_;

    QString path = hash_path ? hash_path->value(node_id) : QString();

    if (option.state & QStyle::State_Selected)
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    else
        painter->setPen(option.palette.color(QPalette::Text));

    if (!path.isEmpty())
        painter->drawText(option.rect.adjusted(2, 0, -2, 0), Qt::AlignLeft | Qt::AlignVCenter, path);
    else
        QStyledItemDelegate::paint(painter, option, index);
}

QSize NodeName::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int node_id { index.sibling(index.row(), static_cast<int>(TreeColumn::kID)).data().toInt() };
    QString path {};

    if (leaf_path_->contains(node_id))
        path = leaf_path_->value(node_id);
    else if (branch_path_->contains(node_id))
        path = branch_path_->value(node_id);
    else
        path = QString();

    QFontMetrics fontMetrics(option.font);
    int width { fontMetrics.size(0, path).width() + 8 };

    return QSize(width, option.rect.height());
}
