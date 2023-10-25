#include "line.h"

#include <QLineEdit>

Line::Line(QObject* parent)
    : QStyledItemDelegate { parent }
{
}

QWidget* Line::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new QLineEdit(parent) };
    return editor;
}

void Line::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<QLineEdit*>(editor) };

    auto string { index.data().toString() };

    cast_editor->setText(string);
}

void Line::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void Line::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<QLineEdit*>(editor) };
    model->setData(index, cast_editor->text());
}
