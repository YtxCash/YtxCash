#include "editnode.h"

#include "ui_editnode.h"

EditNode::EditNode(Node* node, CString* separator, CStringHash* unit_hash, bool node_usage, CString& parent_path, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditNode)
    , node_ { node }
    , separator_ { separator }
    , node_usage_ { node_usage }
    , parent_path_ { parent_path }
{
    ui->setupUi(this);
    IniDialog(unit_hash);
    Data(node);
    IniConnect();
}

EditNode::~EditNode() { delete ui; }

void EditNode::IniDialog(CStringHash* unit_hash)
{
    ui->labNote->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->gridLayout->setContentsMargins(0, 0, 0, 0);
    ui->pBtnOk->setDefault(true);
    ui->pBtnOk->setShortcut(QKeySequence(Qt::Key_Enter));
    ui->plainTextEdit->setTabChangesFocus(true);
    ui->comboUnit->setEditable(true);
    ui->comboUnit->setInsertPolicy(QComboBox::NoInsert);
    ui->lineEditName->setFocus();

    if (!parent_path_.isEmpty())
        parent_path_ += *separator_;

    auto path { parent_path_ };
    if (!node_->name.isEmpty())
        path += node_->name;

    this->setWindowTitle(path);

    QSize mainwindow_size = qApp->activeWindow()->size();
    int width = mainwindow_size.width() * 390 / 1920;
    int height = mainwindow_size.height() * 520 / 1080;
    this->resize(width, height);

    QMap<QString, int> map {};

    for (auto it = unit_hash->cbegin(); it != unit_hash->cend(); ++it)
        map.insert(it.value(), it.key());

    for (auto it = map.cbegin(); it != map.cend(); ++it)
        ui->comboUnit->addItem(it.key(), it.value());
}

void EditNode::IniConnect()
{
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &QDialog::reject, Qt::UniqueConnection);
    connect(ui->pBtnOk, &QPushButton::clicked, this, &EditNode::RCustomAccept, Qt::UniqueConnection);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &EditNode::REditName, Qt::UniqueConnection);
}

void EditNode::Data(Node* tmp_node)
{
    int key { tmp_node->unit };
    int item_index { ui->comboUnit->findData(key) };
    if (item_index != -1)
        ui->comboUnit->setCurrentIndex(item_index);

    if (tmp_node->node_rule)
        ui->rBtnDDCI->setChecked(true);
    else
        ui->rBtnDICD->setChecked(true);

    for (const Node* node : tmp_node->parent->children)
        node_name_list_.emplaceBack(node->name);

    if (tmp_node->name.isEmpty())
        return ui->pBtnOk->setEnabled(false);

    ui->lineEditName->setText(tmp_node->name);
    ui->lineEditCode->setText(tmp_node->code);
    ui->lineEditDescription->setText(tmp_node->description);
    ui->plainTextEdit->setPlainText(tmp_node->note);

    ui->chkBoxBranch->setChecked(tmp_node->branch);
    ui->chkBoxBranch->setEnabled(false);
    if (!node_usage_ && tmp_node->children.isEmpty())
        ui->chkBoxBranch->setEnabled(true);
}

void EditNode::SetData()
{
    node_->name = ui->lineEditName->text().simplified();
    node_->code = ui->lineEditCode->text().simplified();
    node_->description = ui->lineEditDescription->text().simplified();
    node_->unit = ui->comboUnit->currentData().toInt();
    node_->branch = ui->chkBoxBranch->isChecked();
    node_->note = ui->plainTextEdit->toPlainText();

    const bool node_rule = ui->rBtnDICD->isChecked() ? false : true;
    if (node_->base_total != 0 && node_->node_rule != node_rule) {
        node_->base_total = -node_->base_total;
        node_->foreign_total = -node_->foreign_total;
    }
    node_->node_rule = node_rule;
}

void EditNode::RCustomAccept()
{
    SetData();
    accept();
}

void EditNode::REditName(const QString& arg1)
{
    this->setWindowTitle(parent_path_ + arg1);

    if (arg1.isEmpty() || node_name_list_.contains(arg1))
        ui->pBtnOk->setEnabled(false);
    else
        ui->pBtnOk->setEnabled(true);
}
