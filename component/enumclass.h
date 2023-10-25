#ifndef ENUMCLASS_H
#define ENUMCLASS_H

enum class Section { kFinance, kSales, kTask, kNetwork, kProduct, kPurchase };

enum class TableColumn { kID, kPostDate, kCode, kLhsNode, kLhsRate, kDescription, kDocument, kState, kRhsNode, kRhsRate, kDebit, kCredit, kBalance };

enum class TreeColumn { kName, kID, kCode, kDescription, kNote, kNodeRule, kBranch, kUnit, kTotal, kPlaceholder };

enum class Check { kAll, kNone, kReverse };

#endif // ENUMCLASS_H
