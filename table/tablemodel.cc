#include "tablemodel.h"

#include "global/transpool.h"

TableModel::TableModel(const TableInfo* info, TableSql* sql, const Interface* interface, int node_id, bool node_rule, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , sql_ { sql }
    , interface_ { interface }
    , node_id_ { node_id }
    , node_rule_ { node_rule }
{
    sql_->Table(trans_list_, node_id);
    AccumulateBalance(trans_list_, 0, node_rule);
}

TableModel::~TableModel() { RecycleTrans(trans_list_); }

void TableModel::RNodeRule(int node_id, bool node_rule)
{
    if (node_id_ != node_id || node_rule_ == node_rule)
        return;

    for (auto& trans : trans_list_)
        trans->balance = -trans->balance;

    node_rule_ = node_rule;

    emit SResizeColumnToContents(static_cast<int>(TableColumn::kBalance));
}

void TableModel::RUpdateTrans(CSPCTrans& trans, TableColumn column)
{
    if (node_id_ != trans->rhs_node)
        return;

    int row { TransRow(trans->id) };
    if (row == -1)
        return;

    auto old_trans { trans_list_.at(row) };

    switch (column) {
    case TableColumn::kPostDate:
        old_trans->post_date = trans->post_date;
        break;
    case TableColumn::kCode:
        old_trans->code = trans->code;
        break;
    case TableColumn::kDescription:
        old_trans->description = trans->description;
        break;
    case TableColumn::kState:
        old_trans->state = trans->state;
        break;
    case TableColumn::kDocument:
        old_trans->document = trans->document;
        break;
    case TableColumn::kLhsRate:
        old_trans->rhs_rate = trans->lhs_rate;
    case TableColumn::kDebit: {
        old_trans->debit = trans->credit * trans->lhs_rate / old_trans->lhs_rate;
        old_trans->credit = trans->debit * trans->lhs_rate / old_trans->lhs_rate;
        AccumulateBalance(trans_list_, row, node_rule_);
        break;
    }
    default:
        break;
    }
}

void TableModel::RAppendTrans(CSPCTrans& trans)
{
    if (node_id_ != trans->rhs_node)
        return;

    auto new_trans { TransPool::Instance().Allocate() };
    new_trans->post_date = trans->post_date;
    new_trans->id = trans->id;
    new_trans->description = trans->description;
    new_trans->code = trans->code;
    new_trans->document = trans->document;
    new_trans->state = trans->state;

    new_trans->rhs_rate = trans->lhs_rate;
    new_trans->debit = trans->credit * trans->lhs_rate / new_trans->lhs_rate;
    new_trans->credit = trans->debit * trans->lhs_rate / new_trans->lhs_rate;
    new_trans->rhs_node = trans->lhs_node;
    new_trans->lhs_node = trans->rhs_node;

    auto row { trans_list_.size() };

    beginInsertRows(QModelIndex(), row, row);
    trans_list_.emplaceBack(new_trans);
    endInsertRows();

    double previous_balance { row >= 1 ? trans_list_.at(row - 1)->balance : 0.0 };
    new_trans->balance = CalculateBalance(node_rule_, new_trans->debit, new_trans->credit) + previous_balance;
}

void TableModel::RRemoveTrans(const QMultiHash<int, int>& node_trans)
{
    if (!node_trans.contains(node_id_))
        return;

    auto trans_list { node_trans.values(node_id_) };
    RemoveRows(trans_list);
}

bool TableModel::RemoveRows(const QList<int>& trans_id_list)
{
    int min_row {};
    int trans_id {};

    for (int i = 0; i != trans_list_.size(); ++i) {
        trans_id = trans_list_.at(i)->id;

        if (trans_id_list.contains(trans_id)) {
            beginRemoveRows(QModelIndex(), i, i);
            TransPool::Instance().Recycle(trans_list_.takeAt(i));
            endRemoveRows();

            if (min_row == 0)
                min_row = i;

            --i;
        }
    }

    AccumulateBalance(trans_list_, min_row, node_rule_);
    return true;
}

void TableModel::RReplaceTrans(int old_node_id, int new_node_id, const QMultiHash<int, int>& node_trans)
{
    if (node_id_ == old_node_id) {
        RemoveRows(node_trans.values());
        return;
    }

    if (node_id_ == new_node_id) {
        RetrieveTrans(new_node_id, node_trans.values());
        return;
    }

    if (node_trans.contains(node_id_)) {
        auto trans_id_list = node_trans.values(node_id_);

        for (auto& trans : trans_list_) {
            if (trans_id_list.contains(trans->id))
                trans->rhs_node = new_node_id;
        }
    }
}

int TableModel::TransRow(int trans_id) const
{
    int row { 0 };

    for (const SPTrans& trans : trans_list_) {
        if (trans->id == trans_id) {
            return row;
        }
        ++row;
    }
    return -1;
}

void TableModel::UpdateState(Check state)
{
    for (auto& trans : trans_list_) {
        switch (state) {
        case Check::kAll:
            trans->state = true;
            break;
        case Check::kNone:
            trans->state = false;
            break;
        case Check::kReverse:
            trans->state = !trans->state;
            break;
        default:
            break;
        }
    }

    switch (state) {
    case Check::kAll:
        sql_->UpdateState("state", true, state);
        break;
    case Check::kNone:
        sql_->UpdateState("state", false, state);
        break;
    case Check::kReverse:
        sql_->UpdateState("state", true, state);
        break;
    default:
        break;
    }

    int column { static_cast<int>(TableColumn::kState) };
    emit dataChanged(index(0, column), index(rowCount() - 1, column));
}

int TableModel::NodeRow(int node_id) const
{
    int row { 0 };

    for (const SPTrans& trans : trans_list_) {
        if (trans->rhs_node == node_id) {
            return row;
        }
        ++row;
    }
    return -1;
}

void TableModel::RecycleTrans(QList<SPTrans>& list)
{
    for (auto& trans : list) {
        TransPool::Instance().Recycle(trans);
    }

    list.clear();
}

QModelIndex TableModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex TableModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int TableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return trans_list_.size();
}

int TableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_->header.size();
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto trans { trans_list_.at(index.row()) };
    const TableColumn kColumn { index.column() };

    switch (kColumn) {
    case TableColumn::kID:
        return trans->id;
    case TableColumn::kPostDate:
        return trans->post_date;
    case TableColumn::kCode:
        return trans->code;
    case TableColumn::kLhsNode:
        return trans->lhs_node;
    case TableColumn::kLhsRate:
        return trans->lhs_rate;
    case TableColumn::kDescription:
        return trans->description;
    case TableColumn::kRhsNode:
        return trans->rhs_node == 0 ? QVariant() : trans->rhs_node;
    case TableColumn::kRhsRate:
        return trans->rhs_rate;
    case TableColumn::kState:
        return trans->state;
    case TableColumn::kDocument:
        return trans->document.size() == 0 ? QVariant() : QString::number(trans->document.size());
    case TableColumn::kDebit:
        return trans->debit == 0 ? QVariant() : trans->debit;
    case TableColumn::kCredit:
        return trans->credit == 0 ? QVariant() : trans->credit;
    case TableColumn::kBalance:
        return trans->balance;
    default:
        return QVariant();
    }
}

bool TableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const int kRow { index.row() };
    const TableColumn kColumn { index.column() };

    auto trans { trans_list_.at(kRow) };
    int old_rhs_node { trans->rhs_node };

    bool nod_changed { false };
    bool deb_changed { false };
    bool cre_changed { false };
    bool rat_changed { false };
    bool pos_changed { false };
    bool cod_changed { false };
    bool des_changed { false };
    bool sta_changed { false };

    switch (kColumn) {
    case TableColumn::kPostDate:
        pos_changed = UpdatePostDate(trans, value.toString());
        break;
    case TableColumn::kCode:
        cod_changed = UpdateCode(trans, value.toString());
        break;
    case TableColumn::kState:
        sta_changed = UpdateState(trans, value.toBool());
        break;
    case TableColumn::kDescription:
        des_changed = UpdateDescription(trans, value.toString());
        break;
    case TableColumn::kLhsRate:
        rat_changed = UpdateRate(trans, value.toDouble());
        break;
    case TableColumn::kRhsNode:
        nod_changed = UpdateRhsNode(trans, value.toInt());
        break;
    case TableColumn::kDebit:
        deb_changed = UpdateDebit(trans, value.toDouble());
        break;
    case TableColumn::kCredit:
        cre_changed = UpdateCredit(trans, value.toDouble());
        break;
    default:
        return false;
    }

    if (old_rhs_node == 0) {
        if (nod_changed) {
            sql_->InsertTrans(trans);
            AccumulateBalance(trans_list_, kRow, node_rule_);
            emit SAppendTrans(info_->section, trans);
            emit SUpdateTotal(QList<int> { trans->rhs_node, node_id_ });
        }

        emit SResizeColumnToContents(index.column());
        return true;
    }

    if (deb_changed || cre_changed || rat_changed || nod_changed)
        sql_->UpdateTrans(trans);

    if (deb_changed || cre_changed) {
        AccumulateBalance(trans_list_, kRow, node_rule_);
        emit SUpdateTotal(QList<int> { old_rhs_node, node_id_ });
        emit SUpdateTrans(info_->section, trans, TableColumn::kDebit);
    }

    if (nod_changed) {
        emit SRemoveTrans(info_->section, old_rhs_node, trans->id);
        emit SAppendTrans(info_->section, trans);
        emit SUpdateTotal(QList<int> { old_rhs_node, trans->rhs_node, node_id_ });
    }

    if (rat_changed) {
        emit SUpdateTrans(info_->section, trans, TableColumn::kLhsRate);
        emit SUpdateTotal(QList<int> { trans->rhs_node, node_id_ });
    }

    if (pos_changed) {
        sql_->UpdateTrans("post_date", value, trans->id);
        emit SUpdateTrans(info_->section, trans, TableColumn::kPostDate);
    }

    if (des_changed) {
        sql_->UpdateTrans("description", value, trans->id);
        emit SUpdateTrans(info_->section, trans, TableColumn::kDescription);
    }

    if (cod_changed) {
        sql_->UpdateTrans("code", value, trans->id);
        emit SUpdateTrans(info_->section, trans, TableColumn::kCode);
    }

    if (sta_changed) {
        sql_->UpdateTrans("state", value, trans->id);
        emit SUpdateTrans(info_->section, trans, TableColumn::kState);
    }

    emit SSearch();
    emit SResizeColumnToContents(index.column());

    return true;
}

bool TableModel::UpdatePostDate(CSPTrans& trans, CString& value)
{
    if (trans->post_date == value)
        return false;

    trans->post_date = value;
    return true;
}

bool TableModel::UpdateDescription(CSPTrans& trans, CString& value)
{
    if (trans->description == value)
        return false;

    trans->description = value;
    return true;
}

bool TableModel::UpdateCode(CSPTrans& trans, CString& value)
{
    if (trans->code == value)
        return false;

    trans->code = value;
    return true;
}

bool TableModel::UpdateState(CSPTrans& trans, bool value)
{
    if (trans->state == value)
        return false;

    trans->state = value;
    return true;
}

bool TableModel::UpdateRhsNode(CSPTrans& trans, int value)
{
    if (trans->rhs_node == value)
        return false;

    trans->rhs_node = value;
    return true;
}

bool TableModel::UpdateDebit(CSPTrans& trans, double value)
{
    double debit { trans->debit };
    if (debit == value)
        return false;

    double credit { trans->credit };
    double amount { qAbs(value - credit) };

    trans->debit = (value >= credit) ? amount : 0;
    trans->credit = (value < credit) ? amount : 0;

    return true;
}

bool TableModel::UpdateCredit(CSPTrans& trans, double value)
{
    double credit { trans->credit };
    if (credit == value)
        return false;

    double debit { trans->debit };
    double amount { qAbs(value - debit) };

    trans->debit = (value >= debit) ? 0 : amount;
    trans->credit = (value < debit) ? 0 : amount;

    return true;
}

bool TableModel::UpdateRate(CSPTrans& trans, double value)
{
    if (trans->lhs_rate == value || value <= 0)
        return false;

    trans->lhs_rate = value;
    return true;
}

double TableModel::CalculateBalance(bool node_rule, double debit, double credit) { return node_rule ? credit - debit : debit - credit; }

void TableModel::AccumulateBalance(const QList<SPTrans>& list, int row, bool node_rule)
{
    if (row < 0 || row >= list.size() || list.isEmpty())
        return;

    double previous_balance { row >= 1 ? list.at(row - 1)->balance : 0.0 };

    std::accumulate(list.begin() + row, list.end(), previous_balance, [&](double balance, SPTrans trans) {
        trans->balance = CalculateBalance(node_rule, trans->debit, trans->credit) + balance;
        return trans->balance;
    });

    emit SResizeColumnToContents(static_cast<int>(TableColumn::kBalance));
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_->header.at(section);

    return QVariant();
}

void TableModel::sort(int column, Qt::SortOrder order)
{
    // ignore balance column
    if (column < 0 || column >= info_->header.size() - 1)
        return;

    auto Compare = [column, order](CSPTrans& lhs, CSPTrans& rhs) -> bool {
        const TableColumn kColumn { column };

        switch (kColumn) {
        case TableColumn::kPostDate:
            return (order == Qt::AscendingOrder) ? (lhs->post_date < rhs->post_date) : (lhs->post_date > rhs->post_date);
        case TableColumn::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case TableColumn::kLhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_node < rhs->lhs_node) : (lhs->lhs_node > rhs->lhs_node);
        case TableColumn::kLhsRate:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_rate < rhs->lhs_rate) : (lhs->lhs_rate > rhs->lhs_rate);
        case TableColumn::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case TableColumn::kRhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_node < rhs->rhs_node) : (lhs->rhs_node > rhs->rhs_node);
        case TableColumn::kRhsRate:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_rate < rhs->rhs_rate) : (lhs->rhs_rate > rhs->rhs_rate);
        case TableColumn::kState:
            return (order == Qt::AscendingOrder) ? (lhs->state < rhs->state) : (lhs->state > rhs->state);
        case TableColumn::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        case TableColumn::kDebit:
            return (order == Qt::AscendingOrder) ? (lhs->debit < rhs->debit) : (lhs->debit > rhs->debit);
        case TableColumn::kCredit:
            return (order == Qt::AscendingOrder) ? (lhs->credit < rhs->credit) : (lhs->credit > rhs->credit);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(trans_list_.begin(), trans_list_.end(), Compare);
    emit layoutChanged();

    AccumulateBalance(trans_list_, 0, node_rule_);
}

bool TableModel::AppendRow(const QModelIndex& parent)
{
    auto row { trans_list_.size() };
    auto trans { TransPool::Instance().Allocate() };

    trans->lhs_node = node_id_;

    beginInsertRows(parent, row, row);
    trans_list_.emplaceBack(trans);
    endInsertRows();

    return true;
}

bool TableModel::RemoveRow(int row, const QModelIndex& parent)
{
    if (row < 0)
        return false;

    auto trans { trans_list_.at(row) };
    int rhs_node_id { trans->rhs_node };

    beginRemoveRows(parent, row, row);
    trans_list_.removeAt(row);
    endRemoveRows();

    if (rhs_node_id != 0) {
        int trans_id { trans->id };
        emit SRemoveTrans(info_->section, rhs_node_id, trans_id);
        AccumulateBalance(trans_list_, row, node_rule_);
        sql_->RemoveTrans(trans_id);
        emit SUpdateTotal(QList<int> { node_id_, rhs_node_id });
    }

    TransPool::Instance().Recycle(trans);
    emit SSearch();
    return true;
}

bool TableModel::RetrieveTrans(int node_id, const QList<int>& trans_id_list)
{
    auto row { trans_list_.size() };
    auto sptrans_list { sql_->RetrieveTrans(node_id, trans_id_list) };

    beginInsertRows(QModelIndex(), row, row + sptrans_list.size() - 1);
    for (auto& trans : sptrans_list)
        trans_list_.emplace_back(trans);
    endInsertRows();

    AccumulateBalance(trans_list_, row, node_rule_);
    return true;
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const TableColumn kColumn { index.column() };

    switch (kColumn) {
    case TableColumn::kID:
        flags &= ~Qt::ItemIsEditable;
        break;
    case TableColumn::kBalance:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}
