#include "global/signalstation.h"

SignalStation& SignalStation::Instance()
{
    static SignalStation instance;
    return instance;
}

void SignalStation::RegisterModel(Section section_row, int node_id, const TableModel* model)
{
    if (!model_.contains(section_row))
        model_[section_row] = QHash<int, const TableModel*>();

    model_[section_row].insert(node_id, model);
}

void SignalStation::DeregisterModel(Section section, int node_id) { model_[section].remove(node_id); }

void SignalStation::RUpdateTrans(Section section, CSPCTrans& trans, TableColumn column)
{
    auto section_model { model_.value(section) };
    int rhs_node_id { trans->rhs_node };

    auto model { section_model.value(rhs_node_id, nullptr) };
    if (!model)
        return;

    connect(this, &SignalStation::SUpdateTrans, model, &TableModel::RUpdateTrans, Qt::UniqueConnection);
    emit SUpdateTrans(trans, column);
    disconnect(this, &SignalStation::SUpdateTrans, model, &TableModel::RUpdateTrans);
}

void SignalStation::RAppendTrans(Section section, CSPCTrans& trans)
{
    auto section_model { model_.value(section) };
    int rhs_node_id { trans->rhs_node };

    auto model { section_model.value(rhs_node_id, nullptr) };
    if (!model)
        return;

    connect(this, &SignalStation::SAppendTrans, model, &TableModel::RAppendTrans, Qt::UniqueConnection);
    emit SAppendTrans(trans);
    disconnect(this, &SignalStation::SAppendTrans, model, &TableModel::RAppendTrans);
}

void SignalStation::RRemoveTrans(Section section, int node_id, int trans_id)
{
    auto section_model { model_.value(section) };

    auto model { section_model.value(node_id, nullptr) };
    if (!model)
        return;

    QMultiHash<int, int> node_trans {};
    node_trans.insert(node_id, trans_id);

    connect(this, &SignalStation::SRemoveTrans, model, &TableModel::RRemoveTrans, Qt::UniqueConnection);
    emit SRemoveTrans(node_trans);
    disconnect(this, &SignalStation::SRemoveTrans, model, &TableModel::RRemoveTrans);
}
