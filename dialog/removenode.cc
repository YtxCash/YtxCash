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

    auto path { leaf_path_->value(node_id_) };
    QString text {};
    QString informative_text {};

    if (ui->rBtnRemoveTrans->isChecked()) {
        text = tr("Remove Trans");
        informative_text = tr("Will remove %1 and associated transactions, Sure ?").arg(path);
    }

    if (ui->rBtnReplaceTrans->isChecked()) {
        text = tr("Replace Trans");
        informative_text = tr("In associated transactions, %1 will be replaced with %2, Sure ?").arg(path, ui->comboBox->currentText());
    }

    msg.setText(text);
    msg.setInformativeText(informative_text);

    if (msg.exec() == QMessageBox::Ok) {
        if (ui->rBtnRemoveTrans->isChecked())
            emit SRemoveMulti(node_id_);

        if (ui->rBtnReplaceTrans->isChecked()) {
            int new_node_id { ui->comboBox->currentData().toInt() };
            emit SReplaceMulti(node_id_, new_node_id);
        }

        accept();
    }
}

void RemoveNode::IniDialog()
{
    ui->label->setWordWrap(true);
    ui->rBtnReplaceTrans->setChecked(true);
    ui->pBtnCancel->setDefault(true);
    this->setWindowTitle(tr("Remove Node"));

    auto mainwindow_size { qApp->activeWindow()->size() };
    int width { mainwindow_size.width() * 800 / 1920 };
    int height { mainwindow_size.height() * 600 / 1080 };
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

    for (auto it = leaf_path_->cbegin(); it != leaf_path_->cend(); ++it)
        if (it.key() != node_id_)
            ui->comboBox->addItem(it.value(), it.key());

    ui->comboBox->model()->sort(0);
}

void RemoveNode::IniConnect()
{
    connect(ui->pBtnOk, &QPushButton::clicked, this, &RemoveNode::RCustomAccept, Qt::UniqueConnection);
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &QDialog::reject, Qt::UniqueConnection);
    connect(ui->rBtnReplaceTrans, &QRadioButton::toggled, ui->comboBox, &QComboBox::setEnabled);
}
