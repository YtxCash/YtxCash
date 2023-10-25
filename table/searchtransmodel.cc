#include "searchtransmodel.h"

#include "component/enumclass.h"
#include "global/transpool.h"

SearchTransModel::SearchTransModel(const TableInfo* info, SearchSql* sql, QObject* parent)
    : QAbstractItemModel { parent }
    , sql_ { sql }
    , info_ { info }
{
}

SearchTransModel::~SearchTransModel() { RecycleTrans(trans_list_); }

QModelIndex SearchTransModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SearchTransModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SearchTransModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return trans_list_.size();
}

int SearchTransModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_->header.size();
}

QVariant SearchTransModel::data(const QModelIndex& index, int role) const
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
        return trans->rhs_node;
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

QVariant SearchTransModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_->header.at(section);

    return QVariant();
}

void SearchTransModel::sort(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= info_->header.size() - 1)
        return;

    auto Compare = [column, order](const SPTrans& lhs, const SPTrans& rhs) -> bool {
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
}

void SearchTransModel::Query(const QString& text)
{
    if (!trans_list_.isEmpty())
        RecycleTrans(trans_list_);

    QList<SPTrans> list {};
    if (!text.isEmpty())
        list = sql_->Trans(text);

    beginResetModel();
    for (auto& trans : list)
        trans_list_.emplace_back(trans);

    endResetModel();
}

void SearchTransModel::RecycleTrans(QList<SPTrans>& list)
{
    for (auto& transaction : list) {
        TransPool::Instance().Recycle(transaction);
    }

    list.clear();
}
