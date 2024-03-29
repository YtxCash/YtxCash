#ifndef EDITNODE_H
#define EDITNODE_H

#include <QDialog>

#include "component/using.h"

namespace Ui {
class EditNode;
}

class EditNode : public QDialog {
    Q_OBJECT

public:
    EditNode(Node* node, CString* separator, CStringHash* unit_hash, bool node_usage, CString& parent_path, QWidget* parent = nullptr);
    ~EditNode();

private slots:
    void RCustomAccept();
    void REditName(const QString& arg1);

private:
    void IniDialog(CStringHash* currency_map);
    void IniConnect();
    void Data(Node* node);
    void SetData();

private:
    Ui::EditNode* ui;
    Node* node_ {};
    CString* separator_ {};

    bool node_usage_ {};
    QStringList node_name_list_ {};
    QString parent_path_ {};
};

#endif // EDITNODE_H
