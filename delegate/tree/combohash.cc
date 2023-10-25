#include "combohash.h"

#include <widget/combobox.h>

#include <QPainter>

ComboHash::ComboHash(CStringHash* hash, QObject* parent)
    : QStyledItemDelegate { parent }
    , hash_ { hash }
{
}

QWidget* ComboHash::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new ComboBox(parent) };

    QMap<QString, int> map {};

    for (auto it = hash_->cbegin(); it != hash_->cend(); ++it)
        map.insert(it.value(), it.key());

    for (auto it = map.cbegin(); it != map.cend(); ++it)
        editor->addItem(it.key(), it.value());

    return editor;
}

void ComboHash::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<ComboBox*>(editor) };

    int key { index.data().toInt() };
    int item_index { cast_editor->findData(key) };

    if (item_index != -1)
        cast_editor->setCurrentIndex(item_index);
}

void ComboHash::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void ComboHash::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<ComboBox*>(editor) };
    int key { cast_editor->currentData().toInt() };
    model->setData(index, key);
}

void ComboHash::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int key { index.data().toInt() };
    QString path { hash_->value(key) };

    if (path.isEmpty())
        return QStyledItemDelegate::paint(painter, option, index);

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    } else
        painter->setPen(option.palette.color(QPalette::Text));

    painter->drawText(option.rect, Qt::AlignCenter, path);
}
