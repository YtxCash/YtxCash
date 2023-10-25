#include "nodeid.h"

#include <QFontMetrics>
#include <QPainter>

#include "widget/combobox.h"

NodeID::NodeID(CStringHash* leaf_path, int node_id, QObject* parent)
    : QStyledItemDelegate { parent }
    , leaf_path_ { leaf_path }
    , node_id_ { node_id }
{
}

QWidget* NodeID::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new ComboBox(parent) };

    QMultiMap<QString, int> map {};

    for (auto it = leaf_path_->cbegin(); it != leaf_path_->cend(); ++it)
        if (it.key() != node_id_)
            map.insert(it.value(), it.key());

    for (auto it = map.cbegin(); it != map.cend(); ++it)
        editor->addItem(it.key(), it.value());

    return editor;
}

void NodeID::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<ComboBox*>(editor) };

    int key { index.data().toInt() };
    key = key == 0 ? last_insert_ : key;

    int item_index { cast_editor->findData(key) };

    if (item_index != -1)
        cast_editor->setCurrentIndex(item_index);
}

void NodeID::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void NodeID::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<ComboBox*>(editor) };
    int key { cast_editor->currentData().toInt() };
    last_insert_ = key;
    model->setData(index, key);
}

void NodeID::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int key { index.data().toInt() };
    QString path { leaf_path_->value(key) };

    if (path.isEmpty())
        return QStyledItemDelegate::paint(painter, option, index);

    if (option.state & QStyle::State_Selected)
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    else
        painter->setPen(option.palette.color(QPalette::Text));

    painter->drawText(option.rect.adjusted(2, 0, -2, 0), Qt::AlignLeft | Qt::AlignVCenter, path);
}

QSize NodeID::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int key { index.data().toInt() };
    QString path { leaf_path_->value(key) };

    if (path.isEmpty())
        return QSize();

    QFontMetrics fontMetrics(option.font);
    int width { fontMetrics.size(0, path).width() + 8 };

    return QSize(width, option.rect.height());
}
