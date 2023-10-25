#include "searchsql.h"

#include <QSqlError>
#include <QSqlQuery>

#include "global/sqlconnection.h"
#include "global/transpool.h"

SearchSql::SearchSql(CString& node, CString& trans, Section section)
    : db_ { SqlConnection::Instance().Allocate(section) }
    , trans_ { trans }
    , node_ { node }
{
}

QList<int> SearchSql::Node(CString& text)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part = QString("SELECT id "
                        "FROM %1 "
                        "WHERE removed = 0 AND name LIKE '%%2%' ")
                    .arg(node_, text);

    if (text.isEmpty())
        part = QString("SELECT id "
                       "FROM %1 "
                       "WHERE removed = 0 ")
                   .arg(node_);

    query.prepare(part);
    query.bindValue(":text", text);
    if (!query.exec()) {
        qWarning() << "Error in Construct Search Node Table" << query.lastError().text();
        return QList<int>();
    }

    int node_id {};
    QList<int> node_list {};

    while (query.next()) {
        node_id = query.value("id").toInt();
        node_list.emplaceBack(node_id);
    }

    return node_list;
}

QList<SPTrans> SearchSql::Trans(CString& text)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part = QString("SELECT id, lhs_node, lhs_rate, debit, credit, rhs_node, rhs_rate, state, description, code, document, post_date "
                        "FROM %1 "
                        "WHERE (debit = :text OR credit = :text OR description LIKE '%%2%') AND removed = 0 "
                        "ORDER BY post_date ")
                    .arg(trans_, text);

    query.prepare(part);
    query.bindValue(":text", text);
    if (!query.exec()) {
        qWarning() << "Error in Construct Search Transaction model" << query.lastError().text();
        return QList<SPTrans>();
    }

    SPTrans trans {};
    QList<SPTrans> trans_list {};

    while (query.next()) {
        trans = TransPool::Instance().Allocate();
        trans->id = query.value("id").toInt();

        trans->lhs_node = query.value("lhs_node").toInt();
        trans->lhs_rate = query.value("lhs_rate").toDouble();

        trans->debit = query.value("debit").toDouble() / trans->lhs_rate;
        trans->credit = query.value("credit").toDouble() / trans->lhs_rate;

        trans->rhs_node = query.value("rhs_node").toInt();
        trans->rhs_rate = query.value("rhs_rate").toDouble();

        trans->code = query.value("code").toString();
        trans->description = query.value("description").toString();
        trans->document = query.value("document").toString().split(";", Qt::SkipEmptyParts);
        trans->post_date = query.value("post_date").toString();
        trans->state = query.value("state").toBool();
        trans_list.emplace_back(trans);
    }

    return trans_list;
}
