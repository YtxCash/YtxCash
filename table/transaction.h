#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QStringList>

struct Transaction {
    int id {};
    QString post_date {};
    QString code {};
    int lhs_node {};
    double lhs_rate { 1.0 };
    QString description {};
    int rhs_node {};
    double rhs_rate { 1.0 };
    bool state { false };
    QStringList document {};
    double debit {};
    double credit {};
    double balance {};

    Transaction() = default;

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    Transaction& operator=(Transaction&&) = delete;

    void Reset()
    {
        id = 0;
        post_date.clear();
        lhs_node = 0;
        lhs_rate = 1.0;
        debit = 0.0;
        credit = 0.0;
        rhs_node = 0;
        rhs_rate = 1.0;
        state = false;
        description.clear();
        code.clear();
        document.clear();
        balance = 0.0;
    }
};

#endif // TRANSACTION_H
