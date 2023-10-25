#ifndef TREESQL_H
#define TREESQL_H

// for finance, network, product, task

#include <QSqlDatabase>

#include "component/enumclass.h"
#include "component/using.h"

class TreeSql {
public:
    TreeSql() = default;
    TreeSql(CString& node, CString& path, CString& trans, Section section);

    bool Tree(NodeHash& node_hash);
    bool InsertNode(int parent_id, Node* node);
    bool RemoveBranchNode(int node_id);
    bool RemoveLeafNode(int node_id);
    bool UpdateNode(CString& column, const QVariant& value, int node_id);
    bool DragNode(int destination_node_id, int node_id);
    bool ReplaceNode(int old_node_id, int new_node_id); // replace node in transaction table

    bool NodeUsage(int node_id) const;
    QMultiHash<int, int> AssociatedNodeTrans(int node_id) const;
    void LeafTotal(Node* node);

private:
    bool DBTransaction(auto Function);
    void CreateNodeHash(QSqlQuery& query, NodeHash& node_hash);
    void IniRelationship(QSqlQuery& query, const NodeHash& node_hash);

private:
    QSqlDatabase* db_ {};

    QString node_ {}; // SQL database node table name, also used as QSettings section name, be carefull with it
    QString path_ {}; // SQL database node path table name
    QString trans_ {}; // SQL database node transaction table name
};

#endif // TREESQL_H
