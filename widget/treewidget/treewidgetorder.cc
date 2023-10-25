#include "treewidgetorder.h"

#include <QApplication>

#include "component/constvalue.h"
#include "ui_treewidgetorder.h"

TreeWidgetOrder::TreeWidgetOrder(AbstractTreeModel* model, const Info* info, const SectionRule* section_rule, QWidget* parent)
    : AbstractTreeWidget(parent)
    , ui(new Ui::TreeWidgetOrder)
    , model_ { model }
    , info_ { info }
    , section_rule_ { section_rule }
{
    ui->setupUi(this);
    ui->view_->setModel(model);
    ui->dspin_box_dynamic_->setRange(DMIN, DMAX);
    ui->dspin_box_static_->setRange(DMIN, DMAX);
    ui->dateEditEnd->setDate(QDate::currentDate());
    ui->dateEditStart->setDate(QDate::currentDate());
    TreeWidgetOrder::ResetStatus();
}

TreeWidgetOrder::~TreeWidgetOrder() { delete ui; }

void TreeWidgetOrder::SetCurrentIndex(const QModelIndex& index) { ui->view_->setCurrentIndex(index); }

void TreeWidgetOrder::ResetStatus()
{
    static_node_ = model_->GetNode(section_rule_->static_node);
    dynamic_node_lhs_ = model_->GetNode(section_rule_->dynamic_node_lhs);
    dynamic_node_rhs_ = model_->GetNode(section_rule_->dynamic_node_rhs);

    StaticStatus(static_node_);

    ui->dspin_box_dynamic_->setDecimals(section_rule_->value_decimal);
    ui->label_dynamic_->setText(section_rule_->dynamic_label);

    if (dynamic_node_lhs_ && dynamic_node_rhs_ && dynamic_node_lhs_->unit != dynamic_node_rhs_->unit) {
        dynamic_node_lhs_ = nullptr;
        dynamic_node_rhs_ = nullptr;
    }

    if (!dynamic_node_lhs_ && !dynamic_node_rhs_) {
        ui->dspin_box_dynamic_->setPrefix(QString());
        ui->dspin_box_dynamic_->setValue(0.0);
        return;
    }

    DynamicStatus(dynamic_node_lhs_, dynamic_node_rhs_);
}

void TreeWidgetOrder::HideStatus()
{
    ui->dspin_box_dynamic_->setHidden(true);
    ui->dspin_box_static_->setHidden(true);
}

QTreeView* TreeWidgetOrder::View() { return ui->view_; }

QHeaderView* TreeWidgetOrder::Header() { return ui->view_->header(); }

void TreeWidgetOrder::RUpdateDSpinBox()
{
    if (static_node_)
        ui->dspin_box_static_->setValue(static_node_->initial_total);

    if (dynamic_node_lhs_ || dynamic_node_rhs_)
        DynamicStatus(dynamic_node_lhs_, dynamic_node_rhs_);
}

void TreeWidgetOrder::DynamicStatus(const Node* lhs, const Node* rhs)
{
    auto lhs_initial_total { lhs ? lhs->initial_total : 0.0 };
    auto rhs_initial_total { rhs ? rhs->initial_total : 0.0 };

    auto operation { section_rule_->operation.isEmpty() ? PLUS : section_rule_->operation };
    double initial_total { Operate(lhs_initial_total, rhs_initial_total, operation) };

    ui->dspin_box_dynamic_->setPrefix(info_->unit_symbol_hash.value((lhs ? lhs : rhs)->unit, QString()));
    ui->dspin_box_dynamic_->setValue(initial_total);
}

void TreeWidgetOrder::StaticStatus(const Node* node)
{
    ui->dspin_box_static_->setDecimals(section_rule_->value_decimal);
    ui->lable_static_->setText(section_rule_->static_label);

    ui->dspin_box_static_->setPrefix(node ? info_->unit_symbol_hash.value(node->unit, QString()) : QString());
    ui->dspin_box_static_->setValue(node ? node->initial_total : 0.0);
}

double TreeWidgetOrder::Operate(double lhs, double rhs, const QString& operation)
{
    switch (operation.at(0).toLatin1()) {
    case '+':
        return lhs + rhs;
    case '-':
        return lhs - rhs;
    default:
        return 0.0;
    }
}

void TreeWidgetOrder::on_dateEditStart_dateChanged(const QDate& date) { }

void TreeWidgetOrder::on_dateEditEnd_dateChanged(const QDate& date) { }
