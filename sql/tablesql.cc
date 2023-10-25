#include "tablesql.h"

#include <QSqlError>
#include <QSqlQuery>

#include "global/sqlconnection.h"
#include "global/transpool.h"

TableSql::TableSql(CString& trans, Section section)
    : db_ { SqlConnection::Instance().Allocate(section) }
    , trans_ { trans }
{
}

bool TableSql::Table(QList<SPTrans>& list, int node_id)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part = QString("SELECT id, lhs_node, lhs_rate, debit, credit, rhs_node, rhs_rate, state, description, code, document, post_date "
                        "FROM %1 "
                        "WHERE lhs_node = :node_id AND removed = 0 "
                        "UNION ALL "
                        "SELECT id, rhs_node, rhs_rate, credit, debit, lhs_node, lhs_rate, state, description, code, document, post_date "
                        "FROM %1 "
                        "WHERE rhs_node = :node_id AND removed = 0 ")
                    .arg(trans_);

    query.prepare(part);
    query.bindValue(":node_id", node_id);

    if (!query.exec()) {
        qWarning() << "Error in ConstructTable 1st" << query.lastError().text();
        return false;
    }

    SPTrans trans {};

    while (query.next()) {
        trans = TransPool::Instance().Allocate();
        trans->id = query.value("id").toInt();

        trans->lhs_node = query.value("lhs_node").toInt();
        trans->lhs_rate = query.value("lhs_rate").toDouble();

        // convert base unit to froeign unit, rate = foreign / base
        trans->debit = query.value("debit").toDouble() / trans->lhs_rate;
        trans->credit = query.value("credit").toDouble() / trans->lhs_rate;

        trans->rhs_node = query.value("rhs_node").toInt();
        trans->rhs_rate = query.value("rhs_rate").toDouble();

        trans->code = query.value("code").toString();
        trans->description = query.value("description").toString();
        trans->document = query.value("document").toString().split(", ", Qt::SkipEmptyParts);
        trans->post_date = query.value("post_date").toString();
        trans->state = query.value("state").toBool();

        list.emplace_back(trans);
    }

    return true;
}

bool TableSql::InsertTrans(CSPTrans& trans)
{
    QSqlQuery query(*db_);
    auto part = QString("INSERT INTO %1 "
                        "(post_date, lhs_node, lhs_rate, debit, credit, rhs_node, rhs_rate, state, description, code, document) "
                        "VALUES "
                        "(:post_date, :lhs_node, :lhs_rate, :debit, :credit, :rhs_node, :rhs_rate, :state, :description, :code, :document) ")
                    .arg(trans_);

    query.prepare(part);
    query.bindValue(":post_date", trans->post_date);
    query.bindValue(":lhs_node", trans->lhs_node);
    query.bindValue(":lhs_rate", trans->lhs_rate);
    query.bindValue(":debit", trans->debit * trans->lhs_rate);
    query.bindValue(":credit", trans->credit * trans->lhs_rate);
    query.bindValue(":rhs_node", trans->rhs_node);
    query.bindValue(":rhs_rate", trans->rhs_rate);
    query.bindValue(":state", trans->state);
    query.bindValue(":description", trans->description);
    query.bindValue(":code", trans->code);
    query.bindValue(":document", trans->document.join(", "));

    if (!query.exec()) {
        qWarning() << "Failed to insert record in transaction table" << query.lastError().text();
        return false;
    }

    trans->id = query.lastInsertId().toInt();

    return true;
}

bool TableSql::RemoveTrans(int trans_id)
{
    QSqlQuery query(*db_);
    auto part = QString("UPDATE %1 "
                        "SET removed = 1 "
                        "WHERE id = :trans_id ")
                    .arg(trans_);

    query.prepare(part);
    query.bindValue(":trans_id", trans_id);
    if (!query.exec()) {
        qWarning() << "Failed to remove record in transaction table" << query.lastError().text();
        return false;
    }

    return true;
}

bool TableSql::UpdateTrans(CSPCTrans& trans)
{
    double debit { trans->debit * trans->lhs_rate };
    double credit { trans->credit * trans->lhs_rate };

    QSqlQuery query(*db_);

    auto part = QString("UPDATE %1 SET "
                        "lhs_node = :lhs_node, "
                        "lhs_rate = :lhs_node_rate, "
                        "debit = :debit, "
                        "credit = :credit, "
                        "rhs_node = :rhs_node, "
                        "rhs_rate = :rhs_node_rate "
                        "WHERE id = :id ")
                    .arg(trans_);

    query.prepare(part);
    query.bindValue(":lhs_node", trans->lhs_node);
    query.bindValue(":lhs_node_rate", trans->lhs_rate);
    query.bindValue(":debit", debit);
    query.bindValue(":credit", credit);
    query.bindValue(":rhs_node", trans->rhs_node);
    query.bindValue(":rhs_node_rate", trans->rhs_rate);
    query.bindValue(":id", trans->id);

    if (!query.exec()) {
        qWarning() << "Failed to update record in transaction table " << query.lastError().text();
        return false;
    }

    return true;
}

bool TableSql::UpdateTrans(CString& column, const QVariant& value, int trans_id)
{
    QSqlQuery query(*db_);

    auto part = QString("UPDATE %1 "
                        "SET %2 = :value "
                        "WHERE id = :trans_id ")
                    .arg(trans_, column);

    query.prepare(part);
    query.bindValue(":value", value);
    query.bindValue(":trans_id", trans_id);

    if (!query.exec()) {
        qWarning() << "Failed to update record in transaction table " << query.lastError().text();
        return false;
    }

    return true;
}

bool TableSql::UpdateState(CString& column, const QVariant& value, Check state)
{
    QSqlQuery query(*db_);

    auto part = QString("UPDATE %1 "
                        "SET %2 = :value ")
                    .arg(trans_, column);

    if (state == Check::kReverse)
        part = QString("UPDATE %1 "
                       "SET %2 = NOT %2 ")
                   .arg(trans_, column);

    query.prepare(part);
    query.bindValue(":value", value);

    if (!query.exec()) {
        qWarning() << "Failed to update record in transaction table " << query.lastError().text();
        return false;
    }

    return true;
}

QList<SPTrans> TableSql::RetrieveTrans(int node_id, const QList<int>& trans_id_list)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    QStringList list {};

    for (const int& node_id : trans_id_list)
        list.append(QString::number(node_id));

    auto part = QString("SELECT id, lhs_node, lhs_rate, debit, credit, rhs_node, rhs_rate, state, description, code, document, post_date "
                        "FROM %1 "
                        "WHERE id IN (%2) AND lhs_node = :node_id AND removed = 0 "
                        "UNION ALL "
                        "SELECT id, rhs_node, rhs_rate, credit, debit, lhs_node, lhs_rate, state, description, code, document, post_date "
                        "FROM %1 "
                        "WHERE id IN (%2) AND rhs_node = :node_id AND removed = 0 ")
                    .arg(trans_, list.join(", "));

    query.prepare(part);
    query.bindValue(":node_id", node_id);

    if (!query.exec()) {
        qWarning() << "Error in ConstructTable 1st" << query.lastError().text();
        return QList<SPTrans>();
    }

    QList<SPTrans> sptrans_list {};
    SPTrans sptrans {};

    while (query.next()) {
        sptrans = TransPool::Instance().Allocate();
        sptrans->id = query.value("id").toInt();

        sptrans->lhs_node = query.value("lhs_node").toInt();
        sptrans->lhs_rate = query.value("lhs_rate").toDouble();

        sptrans->debit = query.value("debit").toDouble() / sptrans->lhs_rate;
        sptrans->credit = query.value("credit").toDouble() / sptrans->lhs_rate;

        sptrans->rhs_node = query.value("rhs_node").toInt();
        sptrans->rhs_rate = query.value("rhs_rate").toDouble();

        sptrans->code = query.value("code").toString();
        sptrans->description = query.value("description").toString();
        sptrans->document = query.value("document").toStringList();
        sptrans->post_date = query.value("post_date").toString();
        sptrans->state = query.value("state").toBool();

        sptrans_list.emplace_back(sptrans);
    }

    return sptrans_list;
}
