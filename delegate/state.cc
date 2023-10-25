#include "state.h"

#include <QApplication>
#include <QCheckBox>
#include <QMouseEvent>
#include <QPainter>

#include "component/enumclass.h"

State::State(QObject* parent)
    : QStyledItemDelegate { parent }
{
}

QWidget* State::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new QCheckBox(parent) };
    return editor;
}

void State::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<QCheckBox*>(editor) };
    cast_editor->setChecked(index.data().toBool());
}

void State::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    QRect rect { option.rect.bottomLeft(), QSize(16, 16) };
    rect.moveCenter(option.rect.center());
    editor->setGeometry(rect);
}

void State::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    bool state { index.data().toBool() };

    if (index.column() == static_cast<int>(TreeColumn::kBranch) && option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    QRect rect { option.rect.bottomLeft(), QSize(16, 16) };
    rect.moveCenter(option.rect.center());

    QStyleOptionButton check_box {};
    check_box.rect = rect;
    check_box.state = state ? QStyle::State_On : QStyle::State_Off;

    QApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &check_box, painter, option.widget);
}

bool State::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto mouse_event { static_cast<QMouseEvent*>(event) };
        if (option.rect.contains(mouse_event->pos())) {
            bool state { !index.data().toBool() };
            model->setData(index, state);
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
