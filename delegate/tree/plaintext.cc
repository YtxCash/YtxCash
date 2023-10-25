#include "plaintext.h"

#include <QApplication>
#include <QPlainTextEdit>
#include <QRect>

PlainText::PlainText(QObject* parent)
    : QStyledItemDelegate { parent }
{
}

QWidget* PlainText::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new QPlainTextEdit(parent) };
    editor->setTabChangesFocus(true);
    return editor;
}

void PlainText::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<QPlainTextEdit*>(editor) };
    cast_editor->setPlainText(index.data().toString());
}

void PlainText::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    QSize mainwindow_size = qApp->activeWindow()->size();
    int width = mainwindow_size.width() * 150 / 1920;
    int height = mainwindow_size.height() * 200 / 1080;

    QRect rect { option.rect.bottomLeft(), QSize(width, height) };
    editor->setGeometry(rect);
}

void PlainText::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<QPlainTextEdit*>(editor) };
    model->setData(index, cast_editor->toPlainText());
}
