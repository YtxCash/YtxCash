#include "document.h"

#include <QMouseEvent>
#include <QPushButton>

Document::Document(QObject* parent)
    : QStyledItemDelegate { parent }
{
}

QWidget* Document::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new QPushButton(parent) };
    connect(editor, &QPushButton::clicked, this, &Document::RButtonClicked);
    return editor;
}

void Document::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void Document::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto opt { option };
    opt.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, opt, index);
}

void Document::RButtonClicked() { emit SUpdateDocument(); }
