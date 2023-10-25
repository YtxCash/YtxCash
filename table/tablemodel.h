#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>

#include "component/enumclass.h"
#include "component/settings.h"
#include "component/using.h"
#include "sql/tablesql.h"
#include "tableinfo.h"

class TableModel : public QAbstractItemModel {
    Q_OBJECT

public:
    TableModel(const TableInfo* info, TableSql* sql, const Interface* interface, int node_id, bool node_rule, QObject* parent = nullptr);
    ~TableModel();

    TableModel() = delete;
    TableModel(const TableModel&) = delete;
    TableModel(TableModel&&) = delete;
    TableModel& operator=(const TableModel&) = delete;
    TableModel& operator=(TableModel&&) = delete;

signals:
    // send to tree model
    void SUpdateTotal(const QList<int>& node_id_list);
    void SSearch();

    // send to its table view
    void SResizeColumnToContents(int column);

    // send to signal station
    void SUpdateTrans(Section section, CSPCTrans& trans, TableColumn column);
    void SRemoveTrans(Section section, int node_id, int trans_id);
    void SAppendTrans(Section section, CSPCTrans& trans);

public slots:
    // receive from tree model
    void RNodeRule(int node_id, bool node_rule);
    void RReplaceTrans(int old_node_id, int new_node_id, const QMultiHash<int, int>& node_trans);

    // receive from both tree model and signal station
    void RRemoveTrans(const QMultiHash<int, int>& node_trans);

    // receive from signal station
    void RUpdateTrans(CSPCTrans& trans, TableColumn column);
    void RAppendTrans(CSPCTrans& trans);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

public:
    bool AppendRow(const QModelIndex& parent = QModelIndex());
    bool RemoveRow(int row, const QModelIndex& parent = QModelIndex());
    bool RemoveRows(const QList<int>& trans_id_list);

public:
    int NodeRow(int node_id) const;
    int TransRow(int trans_id) const;
    inline SPTrans GetTrans(const QModelIndex& index) const { return trans_list_.at(index.row()); }
    void UpdateState(Check state);

private:
    bool RetrieveTrans(int node_id, const QList<int>& trans_id_list);

    bool UpdatePostDate(CSPTrans& trans, CString& value);
    bool UpdateDescription(CSPTrans& trans, CString& value);
    bool UpdateCode(CSPTrans& trans, CString& value);
    bool UpdateRhsNode(CSPTrans& trans, int value);
    bool UpdateState(CSPTrans& trans, bool value);
    bool UpdateDebit(CSPTrans& trans, double value);
    bool UpdateCredit(CSPTrans& trans, double value);
    bool UpdateRate(CSPTrans& trans, double value);

    double CalculateBalance(bool node_rule, double debit, double credit);
    void AccumulateBalance(const QList<SPTrans>& list, int row, bool node_rule);

    void RecycleTrans(QList<SPTrans>& list);

private:
    QList<SPTrans> trans_list_ {};
    const TableInfo* info_ {};
    TableSql* sql_ {};
    const Interface* interface_ {};

    int node_id_ {};
    bool node_rule_ {};
};

#endif // TABLEMODEL_H
