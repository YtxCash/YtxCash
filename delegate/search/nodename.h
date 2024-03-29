#ifndef NODENAME_H
#define NODENAME_H

#include <QStyledItemDelegate>

#include "component/using.h"

class NodeName : public QStyledItemDelegate {
public:
    NodeName(CStringHash* leaf_path, CStringHash* branch_path, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    CStringHash* leaf_path_ {};
    CStringHash* branch_path_ {};
};

#endif // NODENAME_H
