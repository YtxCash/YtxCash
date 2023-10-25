#include "removenode.h"

#include <QCompleter>
#include <QMessageBox>

#include "ui_removenode.h"

RemoveNode::RemoveNode(CStringHash* leaf_path, int node_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RemoveNode)
    , node_id_ { node_id }
    , leaf_path_ { leaf_path }
{
    ui->setupUi(this);
    IniDialog();
    IniConnect();
}

RemoveNode::~RemoveNode() { delete ui; }

void RemoveNode::RCustomAccept()
{
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Question);
    msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    if (ui->rBtnRemoveTrans->isChecked()) {
        msg.setText(tr("Remove Trans"));
        msg.setInformativeText(tr("Will remove this node and associated transactions, Sure ?"));

        if (msg.exec() == QMessageBox::Ok)
            emit SRemoveTrans(node_id_);
        else
            return;
    }

    if (ui->rBtnReplaceTrans->isChecked()) {
        msg.setText(tr("Replace Trans"));
        msg.setInformativeText(tr("In associated transactions, this node will be replaced with %1, Sure ?").arg(ui->comboBox->currentText()));

        if (msg.exec() == QMessageBox::Ok) {
            int new_node_id { ui->comboBox->currentData().toInt() };
            emit SReplaceTrans(node_id_, new_node_id);
        } else
            return;
    }

    accept();
}

void RemoveNode::IniDialog()
{
    ui->label->setWordWrap(true);
    ui->rBtnReplaceTrans->setChecked(true);
    ui->pBtnCancel->setDefault(true);
    this->setWindowTitle(tr("Remove Node"));

    QSize mainwindow_size = qApp->activeWindow()->size();
    int width = mainwindow_size.width() * 800 / 1920;
    int height = mainwindow_size.height() * 600 / 1080;
    this->resize(width, height);

    auto combo { ui->comboBox };
    combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    combo->setFrame(false);
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);

    auto completer { new QCompleter(combo->model(), combo) };
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    combo->setCompleter(completer);

    QMultiMap<QString, int> map {};

    for (auto it = leaf_path_->cbegin(); it != leaf_path_->cend(); ++it)
        if (it.key() != node_id_)
            map.insert(it.value(), it.key());

    for (auto it = map.cbegin(); it != map.cend(); ++it)
        ui->comboBox->addItem(it.key(), it.value());
}

void RemoveNode::IniConnect()
{
    connect(ui->pBtnOk, &QPushButton::clicked, this, &RemoveNode::RCustomAccept, Qt::UniqueConnection);
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &QDialog::reject, Qt::UniqueConnection);
    connect(ui->rBtnReplaceTrans, &QRadioButton::toggled, ui->comboBox, &QComboBox::setEnabled);
}
