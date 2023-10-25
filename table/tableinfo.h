#ifndef TABLEINFO_H
#define TABLEINFO_H

#include <QStringList>

#include "component/enumclass.h"

struct TableInfo {
    Section section {};
    QString trans {}; // SQL database node transaction table name
    QStringList header {};
};

#endif // TABLEINFO_H
