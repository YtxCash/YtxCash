#include "preferences.h"

#include <QCompleter>
#include <QDir>
#include <QFileDialog>

#include "ui_preferences.h"

Preferences::Preferences(CStringHash* unit_hash, CStringHash* leaf_path, CStringHash* branch_path, CStringList* date_format_list, const Interface& interface,
    const SectionRule& section_rule, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Preferences)
    , leaf_path_ { leaf_path }
    , branch_path_ { branch_path }
    , interface_ { interface }
    , section_rule_ { section_rule }
{
    ui->setupUi(this);
    IniDialog();
    IniConnect();
    IniStringList();

    Data(unit_hash, date_format_list);
}

Preferences::~Preferences() { delete ui; }

void Preferences::IniDialog()
{
    ui->listWidget->setCurrentRow(0);
    ui->stackedWidget->setCurrentIndex(0);
    ui->pBtnOk->setDefault(true);
    this->setWindowTitle(tr("Preferences"));

    QSize mainwindow_size = qApp->activeWindow()->size();
    int width = mainwindow_size.width() * 960 / 1920;
    int height = mainwindow_size.height() * 720 / 1080;
    this->resize(width, height);

    IniCombo(ui->comboBaseUnit);
    IniCombo(ui->comboDateTime);
    IniCombo(ui->comboLanguage);
    IniCombo(ui->comboSeparator);
    IniCombo(ui->comboTheme);
    IniCombo(ui->comboDynamicLhs);
    IniCombo(ui->comboStatic);
    IniCombo(ui->comboOperation);
    IniCombo(ui->comboDynamicRhs);
}

void Preferences::IniCombo(QComboBox* combo)
{
    combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    combo->setFrame(false);
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);

    auto completer { new QCompleter(combo->model(), combo) };
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    combo->setCompleter(completer);
}

void Preferences::IniConnect()
{
    connect(ui->listWidget, &QListWidget::currentRowChanged, ui->stackedWidget, &QStackedWidget::setCurrentIndex, Qt::UniqueConnection);
    connect(ui->pBtnOk, &QPushButton::clicked, this, &Preferences::RCustomAccept, Qt::UniqueConnection);
}

void Preferences::Data(CStringHash* unit_hash, CStringList* date_format_list)
{
    IniComboData(ui->comboTheme, theme_list_, interface_.theme);
    IniComboData(ui->comboLanguage, language_list_, interface_.language);
    IniComboData(ui->comboSeparator, separator_list_, interface_.separator);
    IniComboData(ui->comboDateTime, *date_format_list, interface_.date_format);

    IniComboData(ui->comboBaseUnit, unit_hash, section_rule_.base_unit);
    ui->pBtnDocumentDir->setText(section_rule_.document_dir);
    ui->spinValueDecimal->setValue(section_rule_.value_decimal);
    ui->spinRateDecimal->setValue(section_rule_.rate_decimal);

    ui->lineStatic->setText(section_rule_.static_label);
    IniComboData(ui->comboStatic, leaf_path_, branch_path_, section_rule_.static_node);
    ui->lineDynamic->setText(section_rule_.dynamic_label);
    IniComboData(ui->comboDynamicLhs, leaf_path_, branch_path_, section_rule_.dynamic_node_lhs);
    IniComboData(ui->comboOperation, operation_list_, section_rule_.operation);
    IniComboData(ui->comboDynamicRhs, leaf_path_, branch_path_, section_rule_.dynamic_node_rhs);

    ui->checkHideTime->setChecked(section_rule_.hide_time);
}

void Preferences::IniComboData(QComboBox* combo, CStringHash* hash, int value)
{
    QMap<QString, int> map {};

    for (auto it = hash->cbegin(); it != hash->cend(); ++it)
        map.insert(it.value(), it.key());

    for (auto it = map.cbegin(); it != map.cend(); ++it)
        combo->addItem(it.key(), it.value());

    int item_index { combo->findData(value) };
    if (item_index != -1)
        combo->setCurrentIndex(item_index);
}

void Preferences::IniComboData(QComboBox* combo, CStringHash* leaf, CStringHash* branch, int value)
{
    QMap<QString, int> map {};

    for (auto it = leaf->cbegin(); it != leaf->cend(); ++it)
        map.insert(it.value(), it.key());

    for (auto it = branch->cbegin(); it != branch->cend(); ++it)
        map.insert(it.value(), it.key());

    for (auto it = map.cbegin(); it != map.cend(); ++it)
        combo->addItem(it.key(), it.value());

    int item_index { combo->findData(value) };
    if (item_index != -1)
        combo->setCurrentIndex(item_index);
}

void Preferences::IniComboData(QComboBox* combo, CStringList& list, CString& string)
{
    combo->addItems(list);

    int item_index { combo->findText(string) };
    if (item_index != -1)
        combo->setCurrentIndex(item_index);
}

void Preferences::IniStringList()
{
    language_list_.emplaceBack("en_US");
    language_list_.emplaceBack("zh_CN");

    separator_list_.emplaceBack("-");
    separator_list_.emplaceBack(":");
    separator_list_.emplaceBack("/");

    theme_list_.emplaceBack("Solarized Dark");

    operation_list_.emplaceBack("+");
    operation_list_.emplaceBack("-");
}

void Preferences::on_pBtnApply_clicked() { emit SUpdateSettings(section_rule_, interface_); }

void Preferences::on_comboDateTime_textActivated(const QString& arg1) { interface_.date_format = arg1; }

void Preferences::on_comboSeparator_textActivated(const QString& arg1) { interface_.separator = arg1; }

void Preferences::on_spinValueDecimal_valueChanged(int arg1) { section_rule_.value_decimal = arg1; }

void Preferences::on_spinRateDecimal_valueChanged(int arg1) { section_rule_.rate_decimal = arg1; }

void Preferences::on_comboTheme_textActivated(const QString& arg1) { interface_.theme = arg1; }

void Preferences::on_comboLanguage_textActivated(const QString& arg1) { interface_.language = arg1; }

void Preferences::RCustomAccept()
{
    emit SUpdateSettings(section_rule_, interface_);
    accept();
}

void Preferences::on_pBtnDocumentDir_clicked()
{
    auto dir { ui->pBtnDocumentDir->text() };
    auto default_dir { QFileDialog::getExistingDirectory(this, tr("Select Directory"), QDir::homePath() + "/" + dir) };

    if (!default_dir.isEmpty()) {
        auto relative_path { QDir::home().relativeFilePath(default_dir) };
        section_rule_.document_dir = relative_path;
        ui->pBtnDocumentDir->setText(relative_path);
    }
}

void Preferences::on_comboBaseUnit_activated(int index) { section_rule_.base_unit = ui->comboBaseUnit->itemData(index).toInt(); }

void Preferences::on_pBtnResetDocumentDir_clicked()
{
    section_rule_.document_dir = "";
    ui->pBtnDocumentDir->setText("");
}

void Preferences::on_lineStatic_textChanged(const QString& arg1) { section_rule_.static_label = arg1; }

void Preferences::on_comboStatic_activated(int index)
{
    auto node_id { ui->comboStatic->itemData(index).toInt() };
    section_rule_.static_node = node_id;
}

void Preferences::on_lineDynamic_textChanged(const QString& arg1) { section_rule_.dynamic_label = arg1; }

void Preferences::on_comboDynamicLhs_activated(int index)
{
    auto node_id { ui->comboDynamicLhs->itemData(index).toInt() };
    section_rule_.dynamic_node_lhs = node_id;
}

void Preferences::on_comboOperation_textActivated(const QString& arg1) { section_rule_.operation = arg1; }

void Preferences::on_comboDynamicRhs_activated(int index)
{
    auto node_id { ui->comboDynamicRhs->itemData(index).toInt() };
    section_rule_.dynamic_node_rhs = node_id;
}

void Preferences::on_checkHideTime_toggled(bool checked) { section_rule_.hide_time = checked; }
