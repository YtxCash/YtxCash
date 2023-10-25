#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QStyledItemDelegate>

class Document : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit Document(QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

signals:
    void SUpdateDocument();

private slots:
    void RButtonClicked();
};

#endif // DOCUMENT_H
