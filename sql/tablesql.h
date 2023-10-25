#ifndef TABLESQL_H
#define TABLESQL_H

// for finance, network, product, product

#include <QSqlDatabase>

#include "component/enumclass.h"
#include "component/using.h"

class TableSql {
public:
    TableSql() = default;
    TableSql(CString& trans, Section section);

    bool Table(QList<SPTrans>& list, int node_id);
    bool InsertTrans(CSPTrans& trans);
    bool RemoveTrans(int trans_id);
    bool UpdateTrans(CSPCTrans& trans);
    bool UpdateTrans(CString& column, const QVariant& value, int trans_id);
    bool UpdateState(CString& column, const QVariant& value, Check state);

    // transactions'node_id have been replaced in tree function, retrieve them to table model
    QList<SPTrans> RetrieveTrans(int node_id, const QList<int>& trans_id_list);

private:
    QSqlDatabase* db_ {};
    QString trans_ {}; // SQL database node transaction table name
};

#endif // TABLESQL_H
