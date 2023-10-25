#ifndef SEARCHSQL_H
#define SEARCHSQL_H

#include <QSqlDatabase>

#include "component/enumclass.h"
#include "component/using.h"

class SearchSql {
public:
    SearchSql() = default;
    SearchSql(CString& node, CString& trans, Section section);

    QList<int> Node(CString& text);
    QList<SPTrans> Trans(CString& text);

private:
    QSqlDatabase* db_ {};
    QString trans_ {}; // SQL database node transaction table name
    QString node_ {}; // SQL database node table name, also used as QSettings section name, be carefull with it
};

#endif // SEARCHSQL_H
