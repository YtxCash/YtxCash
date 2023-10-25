#ifndef SIGNALSTATION_H
#define SIGNALSTATION_H

#include "component/enumclass.h"
#include "table/tablemodel.h"
#include "table/transaction.h"

using CSPCTrans = const QSharedPointer<const Transaction>;

class SignalStation : public QObject {
    Q_OBJECT

public:
    static SignalStation& Instance();
    void RegisterModel(Section section, int node_id, const TableModel* model);
    void DeregisterModel(Section section, int node_id);

signals:
    void SUpdateTrans(CSPCTrans& trans, TableColumn column);
    void SAppendTrans(CSPCTrans& trans);
    void SRemoveTrans(const QMultiHash<int, int>& node_trans);

public slots:
    void RUpdateTrans(Section section, CSPCTrans& trans, TableColumn column);
    void RAppendTrans(Section section, CSPCTrans& trans);
    void RRemoveTrans(Section section, int node_id, int trans_id);

private:
    SignalStation() = default;
    ~SignalStation() {};

    SignalStation(const SignalStation&) = delete;
    SignalStation& operator=(const SignalStation&) = delete;
    SignalStation(SignalStation&&) = delete;
    SignalStation& operator=(SignalStation&&) = delete;

private:
    QHash<Section, QHash<int, const TableModel*>> model_ {};
};

#endif // SIGNALSTATION_H
