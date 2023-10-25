#ifndef USING_H
#define USING_H

#include <QHash>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QTableView>

#include "table/transaction.h"
#include "tree/node.h"

using StringHash = QHash<int, QString>;
using CStringHash = const QHash<int, QString>;
using NodeHash = QHash<int, Node*>;
using ViewHash = QHash<int, QTableView*>;

using SPTrans = QSharedPointer<Transaction>;
using SPCTrans = QSharedPointer<const Transaction>;
using CSPTrans = const QSharedPointer<Transaction>;
using CSPCTrans = const QSharedPointer<const Transaction>;

using CString = const QString;
using CStringList = const QStringList;

#endif // USING_H
