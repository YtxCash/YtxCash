#include "mainwindow.h"

#include <QDragEnterEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QLockFile>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QResource>

#include "component/enumclass.h"
#include "delegate/line.h"
#include "delegate/state.h"
#include "delegate/table/datetime.h"
#include "delegate/table/document.h"
#include "delegate/table/nodeid.h"
#include "delegate/table/tablevalue.h"
#include "delegate/tree/combohash.h"
#include "delegate/tree/plaintext.h"
#include "delegate/tree/treevalue.h"
#include "dialog/about.h"
#include "dialog/editnode.h"
#include "dialog/originaldocument.h"
#include "dialog/preferences.h"
#include "dialog/removenode.h"
#include "dialog/search.h"
#include "global/nodepool.h"
#include "global/signalstation.h"
#include "global/sqlconnection.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(CString& directory, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , directory_ { directory }
{
    ResourceFile();
    StaticInterface(directory);

    ui->setupUi(this);
    SetTabWidget();
    SetConnect();
    SetHash();
    SetHeader();
    SetAction();
    SetStatusBar();

    qApp->setWindowIcon(QIcon(":/logo/logo/logo.png"));
    this->setAcceptDrops(true);
    ui->actionAbout->setMenuRole(QAction::AboutRole);
    ui->actionPreferences->setMenuRole(QAction::PreferencesRole);

    RestoreState(ui->splitter, static_interface_, "window", "splitter_state");
    RestoreState(this, static_interface_, "window", "mainwindow_state");
    RestoreGeometry(this, static_interface_, "window", "mainwindow_geometry");

    Recent();

#ifdef Q_OS_WIN
    ui->actionRemove->setShortcut(Qt::Key_Delete);
#elif defined(Q_OS_MACOS)
    ui->actionRemove->setShortcut(Qt::Key_Backspace);
#endif
}

MainWindow::~MainWindow()
{
    SaveState(ui->splitter, static_interface_, "window", "splitter_state");
    SaveState(this, static_interface_, "window", "mainwindow_state");
    SaveGeometry(this, static_interface_, "window", "mainwindow_geometry");
    static_interface_->setValue("start/section", static_cast<int>(start_));

    if (finance_tree_.view) {
        SaveView(finance_view_, finance_data_.tree_info.node, "view");
        SaveState(finance_tree_.view->header(), dynamic_interface_, finance_data_.tree_info.node, "header_state");

        finance_dialog_.clear();
    }

    if (product_tree_.view) {
        SaveView(product_view_, product_data_.tree_info.node, "view");
        SaveState(product_tree_.view->header(), dynamic_interface_, product_data_.tree_info.node, "header_state");

        product_dialog_.clear();
    }

    if (network_tree_.view) {
        SaveView(network_view_, network_data_.tree_info.node, "view");
        SaveState(network_tree_.view->header(), dynamic_interface_, network_data_.tree_info.node, "header_state");

        network_dialog_.clear();
    }

    if (task_tree_.view) {
        SaveView(task_view_, task_data_.tree_info.node, "view");
        SaveState(task_tree_.view->header(), dynamic_interface_, task_data_.tree_info.node, "header_state");

        product_dialog_.clear();
    }

    delete ui;
}

void MainWindow::ROpenFile(CString& file_path)
{
    if (file_path.isEmpty() || file_path == SqlConnection::Instance().DatabaseName())
        return;

    if (SqlConnection::Instance().DatabaseEnable()) {
        QProcess::startDetached(qApp->applicationFilePath(), QStringList() << file_path);
        return;
    }

    if (SqlConnection::Instance().SetDatabaseName(file_path)) {
        QFileInfo file_info(file_path);

        this->setWindowTitle(file_info.baseName());
        SetDynamicInterface(file_path);

        LockFile(file_path);

        sql_ = MainwindowSql(start_);
        SetData();

        switch (start_) {
        case Section::kFinance:
            RFinanceToggled(Qt::Checked);
            break;
        case Section::kNetwork:
            RNetworkToggled(Qt::Checked);
            break;
        case Section::kProduct:
            RProductToggled(Qt::Checked);
            break;
        case Section::kTask:
            RBtnTaskToggled(Qt::Checked);
            break;
        default:
            break;
        }

        QString path { QDir::toNativeSeparators(file_path) };

        if (!recent_.contains(path)) {
            auto menu { ui->menuRecent };
            auto action { new QAction(path, menu) };

            if (menu->isEmpty()) {
                menu->addAction(action);
                SetClearMenuAction();
            } else
                ui->menuRecent->insertAction(ui->actionSeparator, action);

            recent_.emplaceBack(path);
            UpdateRecent();
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
    else
        event->ignore();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    auto mime_data { event->mimeData() };
    if (mime_data->hasUrls()) {
        auto url_list { mime_data->urls() };
        auto file_path { url_list.at(0).toLocalFile() };
        ROpenFile(file_path);
    }
}

void MainWindow::RTreeViewDoubleClicked(const QModelIndex& index)
{
    if (index.column() != 0)
        return;

    if (bool branch { index.siblingAtColumn(static_cast<int>(TreeColumn::kBranch)).data().toBool() })
        return;

    auto node { section_tree_->model->GetNode(index) };
    if (node->id == -1)
        return;

    if (section_view_->contains(node->id)) {
        SwitchTab(node->id);
        return;
    }

    CreateTable(node->name, node->id, node->node_rule);
}

void MainWindow::SwitchTab(int node_id)
{
    auto view { section_view_->value(node_id) };
    ui->tabWidget->setCurrentWidget(view);
}

bool MainWindow::LockFile(CString& file_path)
{
    QFileInfo file_info(file_path);
    auto lock_file_path { file_info.absolutePath() + "/" + file_info.baseName() + ".lock" };

    static QLockFile lock_file { lock_file_path };
    if (!lock_file.tryLock(100))
        return false;

    return true;
}

void MainWindow::RUpdateStatusBarSpin()
{
    auto model { section_tree_->model };

    auto static_node { model->GetNode(section_rule_->static_node) };
    if (static_node) {
        status_bar_.static_spin->setPrefix(section_data_->tree_info.unit_symbol_hash.value(static_node->unit, ""));
        status_bar_.static_spin->setValue(static_node->foreign_total);
    }

    auto dynamic_node_lhs { model->GetNode(section_rule_->dynamic_node_lhs) };
    auto dynamic_node_rhs { model->GetNode(section_rule_->dynamic_node_rhs) };
    if (!dynamic_node_lhs || !dynamic_node_rhs)
        return;

    bool equal_unit { dynamic_node_lhs->unit == dynamic_node_rhs->unit };
    double dynamic_total {};
    auto dynamic_node_lhs_total { equal_unit ? dynamic_node_lhs->foreign_total : dynamic_node_lhs->base_total };
    auto dynamic_node_rhs_total { equal_unit ? dynamic_node_rhs->foreign_total : dynamic_node_rhs->base_total };
    auto operation { section_rule_->operation.isEmpty() ? "+" : section_rule_->operation };

    switch (operation.at(0).toLatin1()) {
    case '+':
        dynamic_total = dynamic_node_lhs_total + dynamic_node_rhs_total;
        break;
    case '-':
        dynamic_total = dynamic_node_lhs_total - dynamic_node_rhs_total;
        break;
    default:
        dynamic_total = 0;
        break;
    }

    equal_unit ? status_bar_.dynamic_spin->setPrefix(section_data_->tree_info.unit_symbol_hash.value(dynamic_node_lhs->unit, ""))
               : status_bar_.dynamic_spin->setPrefix(section_data_->tree_info.unit_symbol_hash.value(section_rule_->base_unit, ""));

    status_bar_.dynamic_spin->setValue(dynamic_total);
}

void MainWindow::UpdateStatusBar()
{
    ui->statusbar->show();
    auto decimal { section_rule_->value_decimal };

    status_bar_.dynamic_spin->setDecimals(decimal);
    status_bar_.static_spin->setDecimals(decimal);

    status_bar_.static_label->setText(section_rule_->static_label);
    status_bar_.dynamic_label->setText(section_rule_->dynamic_label);

    RUpdateStatusBarSpin();
}

void MainWindow::CreateTable(CString& node_name, int node_id, bool node_rule)
{
    auto view { new QTableView(this) };
    auto model { new TableModel(&section_data_->table_info, &section_data_->table_sql, &interface_, node_id, node_rule, this) };
    view->setModel(model);

    int tab_index { ui->tabWidget->addTab(view, node_name) };
    auto path { section_tree_->model->Path(node_id) };

    Tab tab_id { section_data_->tree_info.section, node_id };
    ui->tabWidget->tabBar()->setTabData(tab_index, QVariant::fromValue(tab_id));
    ui->tabWidget->tabBar()->setTabToolTip(tab_index, path);

    ui->tabWidget->setCurrentWidget(view);

    SetView(view);
    view->setColumnHidden(static_cast<int>(TableColumn::kLhsNode), true);
    view->setColumnHidden(static_cast<int>(TableColumn::kRhsRate), true);

    SetConnect(view, model, section_tree_->model);
    CreateDelegate(view, node_id);

    section_view_->insert(node_id, view);
    SignalStation::Instance().RegisterModel(section_data_->tree_info.section, node_id, model);
}

void MainWindow::SetConnect(QTableView* view, TableModel* table, TreeModel* tree)
{
    connect(table, &TableModel::SResizeColumnToContents, view, &QTableView::resizeColumnToContents, Qt::UniqueConnection);
    connect(table, &TableModel::SUpdateTrans, &SignalStation::Instance(), &SignalStation::RUpdateTrans, Qt::UniqueConnection);
    connect(table, &TableModel::SRemoveTrans, &SignalStation::Instance(), &SignalStation::RRemoveTrans, Qt::UniqueConnection);
    connect(table, &TableModel::SAppendTrans, &SignalStation::Instance(), &SignalStation::RAppendTrans, Qt::UniqueConnection);
    connect(table, &TableModel::SUpdateTotal, tree, &TreeModel::RUpdateTotal, Qt::UniqueConnection);
    connect(table, &TableModel::SSearch, tree, &TreeModel::RSearch, Qt::UniqueConnection);

    connect(tree, &TreeModel::SNodeRule, table, &TableModel::RNodeRule, Qt::UniqueConnection);
    connect(tree, &TreeModel::SRemoveTrans, table, &TableModel::RRemoveTrans, Qt::UniqueConnection);
    connect(tree, &TreeModel::SReplaceTrans, table, &TableModel::RReplaceTrans, Qt::UniqueConnection);

    connect(this, &MainWindow::SResizeColumnToContents, view, &QTableView::resizeColumnToContents, Qt::UniqueConnection);
}

void MainWindow::CreateDelegate(QTableView* view, int node_id)
{
    auto node { new NodeID(section_tree_->model->LeafPath(), node_id, view) };
    view->setItemDelegateForColumn(static_cast<int>(TableColumn::kRhsNode), node);

    auto value { new TableValue(&section_rule_->value_decimal, view) };
    view->setItemDelegateForColumn(static_cast<int>(TableColumn::kDebit), value);
    view->setItemDelegateForColumn(static_cast<int>(TableColumn::kCredit), value);
    view->setItemDelegateForColumn(static_cast<int>(TableColumn::kBalance), value);

    auto rate { new TableValue(&section_rule_->rate_decimal, view) };
    view->setItemDelegateForColumn(static_cast<int>(TableColumn::kLhsRate), rate);

    auto date { new DateTime(&interface_.date_format, &section_rule_->hide_time, view) };
    view->setItemDelegateForColumn(static_cast<int>(TableColumn::kPostDate), date);

    auto line { new Line(view) };
    view->setItemDelegateForColumn(static_cast<int>(TableColumn::kDescription), line);
    view->setItemDelegateForColumn(static_cast<int>(TableColumn::kCode), line);

    auto state { new State(view) };
    view->setItemDelegateForColumn(static_cast<int>(TableColumn::kState), state);

    auto document { new Document(view) };
    view->setItemDelegateForColumn(static_cast<int>(TableColumn::kDocument), document);
    connect(document, &Document::SUpdateDocument, this, &MainWindow::RUpdateDocument, Qt::UniqueConnection);
}

Tree MainWindow::CreateTree(const TreeInfo* info, TreeSql* tree_sql, const SectionRule* section_rule, const Interface* interface)
{
    auto view { new QTreeView(this) };
    auto model { new TreeModel(info, tree_sql, section_rule, interface, this) };
    view->setModel(model);

    return { view, model };
}

void MainWindow::CreateDelegate(QTreeView* view)
{
    auto line { new Line(view) };
    view->setItemDelegateForColumn(static_cast<int>(TreeColumn::kDescription), line);
    view->setItemDelegateForColumn(static_cast<int>(TreeColumn::kCode), line);

    auto plain_text { new PlainText(view) };
    view->setItemDelegateForColumn(static_cast<int>(TreeColumn::kNote), plain_text);

    auto value { new TreeValue(&section_rule_->value_decimal, &section_data_->tree_info.unit_symbol_hash, view) };
    view->setItemDelegateForColumn(static_cast<int>(TreeColumn::kTotal), value);

    auto unit { new ComboHash(&section_data_->tree_info.unit_hash, view) };
    view->setItemDelegateForColumn(static_cast<int>(TreeColumn::kUnit), unit);

    auto node_rule { new ComboHash(&node_rule_hash_, view) };
    view->setItemDelegateForColumn(static_cast<int>(TreeColumn::kNodeRule), node_rule);

    auto branch { new State(view) };
    view->setItemDelegateForColumn(static_cast<int>(TreeColumn::kBranch), branch);
}

void MainWindow::SetConnect(QTreeView* view, TreeModel* model)
{
    connect(view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(model, &TreeModel::SFreeView, this, &MainWindow::RFreeView, Qt::UniqueConnection);
    connect(model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);

    connect(model, &TreeModel::SResizeColumnToContents, view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);
    connect(model, &TreeModel::SUpdateStatusBarSpin, this, &MainWindow::RUpdateStatusBarSpin, Qt::UniqueConnection);
}

void MainWindow::PrepInsertNode(QTreeView* view)
{
    auto current_index { view->currentIndex() };
    current_index = current_index.isValid() ? current_index : QModelIndex();

    auto parent_index { current_index.parent() };
    parent_index = parent_index.isValid() ? parent_index : QModelIndex();

    InsertNode(parent_index, current_index.row() + 1);
}

void MainWindow::InsertNode(const QModelIndex& parent, int row)
{
    auto model { section_tree_->model };
    Node* parent_node { model->GetNode(parent) };

    auto node { NodePool::Instance().Allocate() };
    node->node_rule = parent_node->node_rule;
    node->unit = parent_node->unit;
    node->parent = parent_node;

    auto parent_path { model->Path(parent_node->id) };

    auto dialog { new EditNode(node, &interface_.separator, &section_data_->tree_info.unit_hash, false, parent_path, this) };
    if (dialog->exec() == QDialog::Rejected) {
        NodePool::Instance().Recycle(node);
        return;
    }

    if (model->InsertRow(row, parent, node)) {
        auto index = model->index(row, 0, parent);
        section_tree_->view->setCurrentIndex(index);
    }
}

void MainWindow::AppendTrans(QWidget* widget)
{
    auto view { GetTableView(widget) };
    auto model { GetTableModel(view) };
    int row { model->NodeRow(0) };

    if (row == -1) {
        if (model->AppendRow()) {
            auto index { model->index(model->rowCount() - 1, static_cast<int>(TableColumn::kPostDate)) };
            view->setCurrentIndex(index);
        }
        return;
    }

    auto index { model->index(row, static_cast<int>(TableColumn::kRhsNode)) };
    view->setCurrentIndex(index);
}

void MainWindow::RInsertTriggered()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget)
        return;

    if (IsTreeView(widget))
        PrepInsertNode(section_tree_->view);

    if (IsTableView(widget))
        AppendTrans(ui->tabWidget->currentWidget());
}

void MainWindow::RRemoveTriggered()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget)
        return;

    if (IsTreeView(widget))
        RemoveNode(section_tree_->view, section_tree_->model);

    if (IsTableView(widget))
        RemoveTrans(ui->tabWidget->currentWidget());
}

void MainWindow::RemoveNode(QTreeView* view, TreeModel* model)
{
    if (!HasSelection(view))
        return;

    auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    auto node_id { index.siblingAtColumn(static_cast<int>(TreeColumn::kID)).data().toInt() };
    auto node { model->GetNode(node_id) };
    if (!node)
        return;

    if (node->branch) {
        if (node->children.isEmpty())
            model->RemoveRow(index.row(), index.parent());
        else
            RemoveBranchMsg(model, index);

        return;
    }

    if (!section_data_->tree_sql.NodeUsage(node_id)) {
        RemoveAssociatedView(model, index, node_id);
        return;
    }

    auto leaf_path { model->LeafPath() };
    auto dialog { new class RemoveNode(leaf_path, node_id, this) };

    connect(dialog, &RemoveNode::SRemoveTrans, model, &TreeModel::RRemoveTrans, Qt::UniqueConnection);
    connect(dialog, &RemoveNode::SReplaceTrans, model, &TreeModel::RReplaceTrans, Qt::UniqueConnection);

    dialog->exec();
}

void MainWindow::RemoveTrans(QWidget* widget)
{
    auto view { GetTableView(widget) };
    if (!HasSelection(view))
        return;

    auto model { GetTableModel(view) };
    auto index { view->currentIndex() };

    if (model && index.isValid())
        model->RemoveRow(index.row());
}

void MainWindow::RemoveAssociatedView(TreeModel* model, const QModelIndex& index, int node_id)
{
    model->RemoveRow(index.row(), index.parent());
    auto view { section_view_->value(node_id) };

    if (view) {
        FreeView(view);
        section_view_->remove(node_id);
        SignalStation::Instance().DeregisterModel(section_data_->tree_info.section, node_id);
    }
}

void MainWindow::SaveView(const ViewHash& hash, CString& section_name, CString& property)
{
    auto keys { hash.keys() };
    QStringList list {};

    for (const int& node_id : keys)
        list.emplaceBack(QString::number(node_id));

    dynamic_interface_->setValue(QString("%1/%2").arg(section_name, property), list);
}

void MainWindow::RestoreView(CString& section_name, CString& property)
{
    auto variant { dynamic_interface_->value(QString("%1/%2").arg(section_name, property)) };

    QList<int> list {};

    if (variant.isValid() && variant.canConvert<QStringList>()) {
        auto variant_list { variant.value<QStringList>() };
        for (const auto& node_id : variant_list)
            list.emplaceBack(node_id.toInt());
    }

    auto model { section_tree_->model };
    const Node* node {};

    for (const int& node_id : list) {
        node = model->GetNode(node_id);
        if (node) {
            CreateTable(node->name, node->id, node->node_rule);
        }
    }
}

void MainWindow::Recent()
{
    recent_ = static_interface_->value("recent/file").toStringList();

    auto recent_menu { ui->menuRecent };
    QStringList valid {};

    for (auto& file : recent_) {
        if (QFile::exists(file)) {
            auto action { recent_menu->addAction(file) };
            valid.emplaceBack(file);

            connect(action, &QAction::triggered, [file, this]() { ROpenFile(file); });
        }
    }

    if (recent_ != valid) {
        recent_ = valid;
        UpdateRecent();
    }

    SetClearMenuAction();
}

void MainWindow::RemoveBranchMsg(TreeModel* model, const QModelIndex& index)
{
    QMessageBox msg {};
    msg.setIcon(QMessageBox::Question);
    msg.setText(tr("Remove Branch"));
    msg.setInformativeText(tr("This branch will be deleted, children are promoted to current layer"));
    msg.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);

    if (msg.exec() == QMessageBox::Ok)
        model->RemoveRow(index.row(), index.parent());
}

void MainWindow::RTabCloseRequested(int index)
{
    if (index == 0)
        return;

    int node_id { ui->tabWidget->tabBar()->tabData(index).value<Tab>().node };
    auto view { section_view_->value(node_id) };

    FreeView(view);
    section_view_->remove(node_id);

    SignalStation::Instance().DeregisterModel(section_data_->table_info.section, node_id);
}

template <typename T>
    requires InheritQAbstractItemView<T>
void MainWindow::FreeView(T*& view)
{
    if (view) {
        if (auto model = view->model()) {
            view->setModel(nullptr);
            delete model;
            model = nullptr;
        }

        delete view;
        view = nullptr;
    }
}

void MainWindow::SetTabWidget()
{
    ui->tabWidget->tabBar()->setDocumentMode(true);
    ui->tabWidget->tabBar()->setExpanding(false);
    ui->tabWidget->setMovable(true);
    ui->tabWidget->setTabsClosable(true);
    ui->tabWidget->setElideMode(Qt::ElideNone);
    ui->tabWidget->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);

    ui->rBtnPurchase->setHidden(true);
    ui->rBtnSales->setHidden(true);

    ui->splitter->setCollapsible(1, true);

    start_ = Section(static_interface_->value("start/section", 0).toInt());

    switch (start_) {
    case Section::kFinance:
        ui->rBtnFinance->setChecked(true);
        break;
    case Section::kNetwork:
        ui->rBtnNetwork->setChecked(true);
        break;
    case Section::kProduct:
        ui->rBtnProduct->setChecked(true);
        break;
    case Section::kTask:
        ui->rBtnTask->setChecked(true);
        break;
    default:
        break;
    }
}

void MainWindow::SetView(QTableView* view)
{
    view->setSortingEnabled(true);
    view->sortByColumn(static_cast<int>(TableColumn::kPostDate), Qt::AscendingOrder);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);
    view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::CurrentChanged);
    view->setColumnHidden(static_cast<int>(TableColumn::kID), true);

    auto h_header { view->horizontalHeader() };
    auto count { view->model()->columnCount() };

    for (int column = 0; column != count; ++column) {
        TableColumn x { column };

        switch (x) {
        case TableColumn::kDescription:
            h_header->setSectionResizeMode(column, QHeaderView::Stretch);
            break;
        default:
            h_header->setSectionResizeMode(column, QHeaderView::ResizeToContents);
            break;
        }
    }

    auto v_header { view->verticalHeader() };
    v_header->setDefaultSectionSize(24);
    v_header->setSectionResizeMode(QHeaderView::Fixed);
    view->verticalHeader()->setHidden(true);
}

void MainWindow::SetConnect()
{
    connect(ui->actionInsert, &QAction::triggered, this, &MainWindow::RInsertTriggered, Qt::UniqueConnection);
    connect(ui->actionRemove, &QAction::triggered, this, &MainWindow::RRemoveTriggered, Qt::UniqueConnection);
    connect(ui->actionAppend, &QAction::triggered, this, &MainWindow::RPrepAppendTriggered, Qt::UniqueConnection);
    connect(ui->actionJump, &QAction::triggered, this, &MainWindow::RJumpTriggered, Qt::UniqueConnection);
    connect(ui->actionEdit, &QAction::triggered, this, &MainWindow::REditTriggered, Qt::UniqueConnection);
    connect(ui->actionSearch, &QAction::triggered, this, &MainWindow::RSearchTriggered, Qt::UniqueConnection);
    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::RPreferencesTriggered, Qt::UniqueConnection);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::RAboutTriggered, Qt::UniqueConnection);
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::RNewTriggered, Qt::UniqueConnection);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::ROpenTriggered, Qt::UniqueConnection);
    connect(ui->actionClearMenu, &QAction::triggered, this, &MainWindow::RClearMenuTriggered, Qt::UniqueConnection);

    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::RTabCloseRequested, Qt::UniqueConnection);
    connect(ui->tabWidget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::RTabBarDoubleClicked, Qt::UniqueConnection);

    connect(ui->rBtnProduct, &QRadioButton::toggled, this, &MainWindow::RProductToggled, Qt::UniqueConnection);
    connect(ui->rBtnFinance, &QRadioButton::toggled, this, &MainWindow::RFinanceToggled, Qt::UniqueConnection);
    connect(ui->rBtnTask, &QRadioButton::toggled, this, &MainWindow::RBtnTaskToggled, Qt::UniqueConnection);
    connect(ui->rBtnNetwork, &QRadioButton::toggled, this, &MainWindow::RNetworkToggled, Qt::UniqueConnection);

    connect(ui->actionCheckAll, &QAction::triggered, this, &MainWindow::RUpdateState, Qt::UniqueConnection);
    connect(ui->actionCheckNone, &QAction::triggered, this, &MainWindow::RUpdateState, Qt::UniqueConnection);
    connect(ui->actionCheckReverse, &QAction::triggered, this, &MainWindow::RUpdateState, Qt::UniqueConnection);
}

void MainWindow::SetFinanceData()
{
    auto section { Section::kFinance };
    auto node { "finance" };
    auto path { "finance_path" };
    auto trans { "finance_transaction" };

    auto& tree_info { finance_data_.tree_info };
    tree_info.section = section;
    tree_info.node = node;

    QStringList unit_list { "CNY", "HKD", "USD", "GBP", "JPY", "CAD", "AUD", "EUR" };
    auto& unit_hash { tree_info.unit_hash };

    QStringList unit_symbol_list { "¥", "$", "$", "£", "¥", "$", "$", "€" };
    auto& unit_symbol_hash { tree_info.unit_symbol_hash };

    for (int i = 0; i != unit_list.size(); ++i) {
        unit_hash.insert(i, unit_list.at(i));
        unit_symbol_hash.insert(i, unit_symbol_list.at(i));
    }

    sql_.QuerySectionRule(finance_rule_, section);

    finance_data_.table_info.section = section;
    finance_data_.table_info.trans = trans;

    finance_data_.tree_sql = TreeSql(node, path, trans, section);
    finance_data_.table_sql = TableSql(trans, section);
    finance_data_.search_sql = SearchSql(node, trans, section);
}

void MainWindow::SetProductData()
{
    auto section { Section::kProduct };
    auto node { "product" };
    auto path { "product_path" };
    auto trans { "product_transaction" };

    auto& tree_info { product_data_.tree_info };
    tree_info.section = Section::kProduct;
    tree_info.node = "product";

    QStringList unit_list { "SF", "PC", "BX" };
    auto& unit_hash { tree_info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(product_rule_, section);

    product_data_.table_info.section = section;
    product_data_.table_info.trans = trans;

    product_data_.tree_sql = TreeSql(node, path, trans, section);
    product_data_.table_sql = TableSql(trans, section);
    product_data_.search_sql = SearchSql(node, trans, section);
}

void MainWindow::SetNetworkData()
{
    auto section { Section::kNetwork };
    auto node { "network" };
    auto path { "network_path" };
    auto trans { "network_transaction" };

    auto& tree_info { network_data_.tree_info };
    tree_info.section = section;
    tree_info.node = node;

    QStringList unit_list { "C", "V", "E" };
    auto& unit_hash { tree_info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(network_rule_, section);

    network_data_.table_info.section = section;
    network_data_.table_info.trans = trans;

    network_data_.tree_sql = TreeSql(node, path, trans, section);
    network_data_.table_sql = TableSql(trans, section);
    network_data_.search_sql = SearchSql(node, trans, section);
}

void MainWindow::SetTaskData()
{
    auto section { Section::kTask };
    auto node { "task" };
    auto path { "task_path" };
    auto trans { "task_transaction" };

    auto& tree_info { task_data_.tree_info };
    tree_info.section = section;
    tree_info.node = node;

    QStringList unit_list { "PER", "SF", "PC", "BX", "PROJ" };
    auto& unit_hash { tree_info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(task_rule_, section);

    task_data_.table_info.section = section;
    task_data_.table_info.trans = trans;

    task_data_.tree_sql = TreeSql(node, path, trans, section);
    task_data_.table_sql = TableSql(trans, section);
    task_data_.search_sql = SearchSql(node, trans, section);
}

void MainWindow::SetHash()
{
    node_rule_hash_.insert(0, "DICD");
    node_rule_hash_.insert(1, "DDCI");

    date_format_list_.emplaceBack("yyyy-MM-dd HH:mm");
}

void MainWindow::SetData()
{
    SetFinanceData();
    SetProductData();
    SetNetworkData();
    SetTaskData();
}

void MainWindow::SetHeader()
{
    finance_data_.tree_info.header = { tr("Account"), tr("ID"), tr("Code"), tr("Description"), tr("Note"), tr("R"), tr("B"), tr("U"), tr("Total"), "" };
    finance_data_.table_info.header = { tr("ID"), tr("PostDate"), tr("Code"), tr("LhsNode"), tr("LhsRate"), tr("Description"), tr("D"), tr("S"), tr("RhsNode"),
        tr("RhsRate"), tr("Debit"), tr("Credit"), tr("Balance") };

    product_data_.tree_info.header = { tr("Variety"), tr("ID"), tr("Code"), tr("Description"), tr("Note"), tr("R"), tr("B"), tr("U"), tr("Total"), "" };
    product_data_.table_info.header = { tr("ID"), tr("PostDate"), tr("Code"), tr("LhsNode"), tr("LhsRate"), tr("Description"), tr("D"), tr("S"), tr("RhsNode"),
        tr("RhsRate"), tr("Debit"), tr("Credit"), tr("Balance") };

    network_data_.tree_info.header = { tr("Name"), tr("ID"), tr("Code"), tr("Description"), tr("Note"), tr("R"), tr("B"), tr("U"), tr("Total"), "" };
    network_data_.table_info.header = { tr("ID"), tr("PostDate"), tr("Code"), tr("LhsNode"), tr("LhsRate"), tr("Description"), tr("D"), tr("S"), tr("RhsNode"),
        tr("RhsRate"), tr("Debit"), tr("Credit"), tr("Balance") };

    task_data_.tree_info.header = { tr("Name"), tr("ID"), tr("Code"), tr("Description"), tr("Note"), tr("R"), tr("B"), tr("U"), tr("Process"), "" };
    task_data_.table_info.header = { tr("ID"), tr("PostDate"), tr("Code"), tr("LhsNode"), tr("LhsRate"), tr("Description"), tr("D"), tr("S"), tr("RhsNode"),
        tr("RhsRate"), tr("Debit"), tr("Credit"), tr("Balance") };
}

void MainWindow::SetAction()
{
    ui->actionInsert->setIcon(QIcon(":/solarized_dark/solarized_dark/insert.png"));
    ui->actionEdit->setIcon(QIcon(":/solarized_dark/solarized_dark/edit.png"));
    ui->actionRemove->setIcon(QIcon(":/solarized_dark/solarized_dark/remove.png"));
    ui->actionAbout->setIcon(QIcon(":/solarized_dark/solarized_dark/about.png"));
    ui->actionAppend->setIcon(QIcon(":/solarized_dark/solarized_dark/append.png"));
    ui->actionJump->setIcon(QIcon(":/solarized_dark/solarized_dark/jump.png"));
    ui->actionPreferences->setIcon(QIcon(":/solarized_dark/solarized_dark/settings.png"));
    ui->actionSearch->setIcon(QIcon(":/solarized_dark/solarized_dark/search.png"));
    ui->actionNew->setIcon(QIcon(":/solarized_dark/solarized_dark/new.png"));
    ui->actionOpen->setIcon(QIcon(":/solarized_dark/solarized_dark/open.png"));

    ui->actionCheckAll->setProperty("check", static_cast<int>(Check::kAll));
    ui->actionCheckNone->setProperty("check", static_cast<int>(Check::kNone));
    ui->actionCheckReverse->setProperty("check", static_cast<int>(Check::kReverse));

    ui->rBtnFinance->setProperty("section", static_cast<int>(Section::kFinance));
    ui->rBtnNetwork->setProperty("section", static_cast<int>(Section::kNetwork));
    ui->rBtnProduct->setProperty("section", static_cast<int>(Section::kProduct));
    ui->rBtnTask->setProperty("section", static_cast<int>(Section::kTask));
}

void MainWindow::SetView(QTreeView* view)
{
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setDragDropMode(QAbstractItemView::InternalMove);
    view->setEditTriggers(QAbstractItemView::DoubleClicked);
    view->setDropIndicatorShown(true);
    view->setSortingEnabled(true);
    view->setColumnHidden(static_cast<int>(TreeColumn::kID), true);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setExpandsOnDoubleClick(true);

    auto header { view->header() };
    auto count { view->model()->columnCount() };

    for (int column = 0; column != count; ++column) {
        TreeColumn x { column };

        switch (x) {
        case TreeColumn::kDescription:
            header->setSectionResizeMode(column, QHeaderView::Stretch);
            break;
        default:
            header->setSectionResizeMode(column, QHeaderView::ResizeToContents);
            break;
        }
    }

    header->setDefaultAlignment(Qt::AlignCenter);
    header->setStretchLastSection(true);
}

void MainWindow::RPrepAppendTriggered()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTreeView(widget))
        return;

    auto view { section_tree_->view };
    if (!HasSelection(view))
        return;

    auto parent_index { view->currentIndex() };
    if (!parent_index.isValid())
        return;

    bool branch { parent_index.siblingAtColumn(static_cast<int>(TreeColumn::kBranch)).data().toBool() };
    if (!branch)
        return;

    InsertNode(parent_index, 0);
}

void MainWindow::RJumpTriggered()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTableView(widget))
        return;

    auto view { GetTableView(widget) };
    if (!HasSelection(view))
        return;

    auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { GetTableModel(view) };
    auto trans { model->GetTrans(index) };
    int rhs_node_id { trans->rhs_node };
    if (rhs_node_id == 0)
        return;

    if (section_view_->contains(rhs_node_id))
        SwitchTab(rhs_node_id);
    else {
        auto rhs_node { section_tree_->model->GetNode(rhs_node_id) };
        if (!rhs_node)
            return;

        CreateTable(rhs_node->name, rhs_node_id, rhs_node->node_rule);
    }

    auto rhs_view { section_view_->value(rhs_node_id) };
    auto rhs_model { GetTableModel(rhs_view) };

    int trans_id { trans->id };
    int rhs_row { rhs_model->TransRow(trans_id) };
    if (rhs_row == -1)
        return;
    auto rhs_trans_index { rhs_model->index(rhs_row, 0, QModelIndex()) };
    rhs_view->setCurrentIndex(rhs_trans_index);
}

void MainWindow::RTreeViewCustomContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);

    auto menu = new QMenu(this);
    menu->addAction(ui->actionInsert);
    menu->addAction(ui->actionEdit);
    menu->addAction(ui->actionAppend);
    menu->addAction(ui->actionRemove);

    menu->exec(QCursor::pos());
}

void MainWindow::REditTriggered()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTreeView(widget))
        return;

    auto view { section_tree_->view };
    if (!HasSelection(view))
        return;

    auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { section_tree_->model };
    auto node { model->GetNode(index) };
    if (node->id == -1)
        return;

    Node tmp_node { *node };
    bool node_usage { section_data_->tree_sql.NodeUsage(node->id) };
    auto parent_path { model->Path(node->parent->id) };

    auto dialog { new EditNode(&tmp_node, &interface_.separator, &section_data_->tree_info.unit_hash, node_usage, parent_path, this) };
    if (dialog->exec() == QDialog::Accepted)
        model->UpdateNode(&tmp_node);
}

template <typename T>
    requires InheritQAbstractItemView<T>
bool MainWindow::HasSelection(T* view)
{
    if (!view)
        return false;
    auto selection_model = view->selectionModel();
    return selection_model && selection_model->hasSelection();
}

void MainWindow::RUpdateDocument()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTableView(widget))
        return;

    auto view { GetTableView(widget) };
    if (!HasSelection(view))
        return;

    auto trans_index { view->currentIndex() };
    if (!trans_index.isValid())
        return;

    auto document_dir { section_rule_->document_dir };
    auto model { GetTableModel(view) };
    auto trans { model->GetTrans(trans_index) };

    auto dialog { new OriginalDocument(section_data_->tree_info.section, trans, document_dir, this) };
    connect(dialog, &OriginalDocument::SUpdateTrans, &SignalStation::Instance(), &SignalStation::RUpdateTrans, Qt::UniqueConnection);

    if (dialog->exec() == QDialog::Accepted)
        section_data_->table_sql.UpdateTrans("document", trans->document.join(", "), trans->id);
}

void MainWindow::RUpdateName(const Node* node)
{
    auto model { section_tree_->model };
    int node_id { node->id };
    auto tab_bar { ui->tabWidget->tabBar() };
    int count { ui->tabWidget->count() };

    if (!node->branch) {
        if (!section_view_->contains(node_id))
            return;

        auto path { model->Path(node_id) };

        for (int index = 0; index != count; ++index) {
            if (tab_bar->tabData(index).value<Tab>().node == node_id) {
                tab_bar->setTabText(index, node->name);
                tab_bar->setTabToolTip(index, path);
            }
        }
    }

    if (node->branch) {
        QQueue<const Node*> queue {};
        queue.enqueue(node);

        QList<int> list {};
        while (!queue.isEmpty()) {
            auto queue_node = queue.dequeue();

            if (queue_node->branch)
                for (const auto* child : queue_node->children)
                    queue.enqueue(child);
            else
                list.emplaceBack(queue_node->id);
        }

        int node_id {};
        QString path {};

        for (int index = 0; index != count; ++index) {
            node_id = tab_bar->tabData(index).value<Tab>().node;

            if (list.contains(node_id)) {
                path = model->Path(node_id);
                tab_bar->setTabToolTip(index, path);
            }
        }
    }
}

void MainWindow::RUpdateSettings(const SectionRule& section_rule, const Interface& interface)
{
    if (interface_ != interface)
        UpdateInterface(interface);

    if (*section_rule_ != section_rule) {
        bool update_base_unit { section_rule_->base_unit == section_rule.base_unit ? false : true };
        *section_rule_ = section_rule;

        if (update_base_unit) {
            auto model { section_tree_->model };
            auto node_hash { model->GetNodeHash() };
            node_hash->value(-1)->unit = section_rule.base_unit;

            for (auto node : *node_hash)
                model->UpdateBranchUnit(node);
        }

        UpdateStatusBar();
        sql_.UpdateSectionRule(section_rule, section_data_->tree_info.section);

        emit SResizeColumnToContents(static_cast<int>(TableColumn::kPostDate));
        emit SResizeColumnToContents(static_cast<int>(TableColumn::kDebit));
        emit SResizeColumnToContents(static_cast<int>(TableColumn::kCredit));
        emit SResizeColumnToContents(static_cast<int>(TableColumn::kBalance));
        emit SResizeColumnToContents(static_cast<int>(TableColumn::kLhsRate));
    }
}
void MainWindow::RFreeView(int node_id)
{
    auto view { section_view_->value(node_id) };

    if (view) {
        FreeView(view);
        section_view_->remove(node_id);
        SignalStation::Instance().DeregisterModel(section_data_->tree_info.section, node_id);
    }
}

void MainWindow::UpdateInterface(const Interface& interface)
{
    auto language { interface.language };
    if (interface_.language != language) {
        if (language == "en_US") {
            qApp->removeTranslator(&cash_translator_);
            qApp->removeTranslator(&base_translator_);
        } else {
            QString cash_language { ":/I18N/I18N/YtxCash_" + language + ".qm" };
            if (cash_translator_.load(cash_language)) {
                qApp->installTranslator(&cash_translator_);
            }

            QString base_language { ":/I18N/I18N/qtbase_" + language + ".qm" };
            if (base_translator_.load(base_language))
                qApp->installTranslator(&base_translator_);
        }

        ui->retranslateUi(this);
        SetHeader();
        UpdateTranslate();
    }

    auto separator { interface.separator };
    auto old_separator { interface_.separator };

    if (old_separator != separator) {
        if (finance_tree_.model)
            finance_tree_.model->UpdateSeparator(separator);

        if (network_tree_.model)
            network_tree_.model->UpdateSeparator(separator);

        if (product_tree_.model)
            product_tree_.model->UpdateSeparator(separator);

        QRegularExpression pat { old_separator };
        QString path {};
        auto widget { ui->tabWidget };
        int count { ui->tabWidget->count() };

        for (int index = 0; index != count; ++index) {
            path = widget->tabToolTip(index).replace(pat, separator);
            widget->setTabToolTip(index, path);
        }
    }

    interface_ = interface;

    static_interface_->beginGroup("interface");
    static_interface_->setValue("language", interface.language);
    static_interface_->setValue("separator", interface.separator);
    static_interface_->setValue("date_format", interface.date_format);
    static_interface_->endGroup();
}

void MainWindow::UpdateTranslate()
{
    QWidget* widget {};
    Tab tab_id {};
    auto tab_widget { ui->tabWidget };
    auto tab_bar { ui->tabWidget->tabBar() };
    int count { ui->tabWidget->count() };

    for (int index = 0; index != count; ++index) {
        widget = tab_widget->widget(index);
        tab_id = tab_bar->tabData(index).value<Tab>();

        if (IsTreeView(widget)) {
            Section section { tab_id.section };
            switch (section) {
            case Section::kFinance:
                tab_widget->setTabText(index, tr("Finance"));
                break;
            case Section::kNetwork:
                tab_widget->setTabText(index, tr("Network"));
                break;
            case Section::kProduct:
                tab_widget->setTabText(index, tr("Product"));
                break;
            default:
                break;
            }
        }

        if (IsTableView(widget) && tab_id.node == -1)
            tab_widget->setTabText(index, tr("General Journal"));
    }
}

void MainWindow::UpdateRecent() { static_interface_->setValue("recent/file", recent_); }

void MainWindow::StaticInterface(CString& directory)
{
    static_interface_path_ = directory + "/YtxCash.ini";
    static QSettings static_interface(static_interface_path_, QSettings::IniFormat);
    static_interface_ = &static_interface;

    static_interface.beginGroup("interface");
    interface_.language = static_interface.value("language", "zh_CN").toString();
    interface_.theme = static_interface.value("theme", "Solarized Dark").toString();
    interface_.date_format = static_interface.value("date_format", "yyyy-MM-dd HH:mm").toString();
    interface_.separator = static_interface.value("separator", "-").toString();
    static_interface.endGroup();

    QString cash_language { ":/I18N/I18N/YtxCash_" + interface_.language + ".qm" };
    if (cash_translator_.load(cash_language))
        qApp->installTranslator(&cash_translator_);

    QString base_language { ":/I18N/I18N/qtbase_" + interface_.language + ".qm" };
    if (base_translator_.load(base_language))
        qApp->installTranslator(&base_translator_);

    QString theme { "file:///:/theme/theme/" + interface_.theme + ".qss" };
    qApp->setStyleSheet(theme);
}

void MainWindow::SetDynamicInterface(CString& path)
{
    QFileInfo info(path);
    dynamic_interface_path_ = directory_ + "/" + info.baseName() + ".ini";

    static QSettings dynamic_interface(dynamic_interface_path_, QSettings::IniFormat);
    dynamic_interface_ = &dynamic_interface;
}

void MainWindow::ResourceFile()
{
    QString path {};

#ifdef Q_OS_WIN
    path = QCoreApplication::applicationDirPath() + "/Resources";

    if (QDir dir(path); !dir.exists()) {
        if (!QDir::home().mkpath(path)) {
            qDebug() << "Failed to create directory:" << path;
            return;
        }
    }

    path += "/resource.brc";

#if 0
    QString command { "D:/Qt/6.5.3/mingw_64/bin/rcc.exe" };
    QStringList arguments {};
    arguments << "-binary"
              << "E:/YtxCash/resource/resource.qrc"
              << "-o" << path;

    QProcess process {};

    // 启动终端并执行命令
    process.start(command, arguments);
    process.waitForFinished();
#endif

#elif defined(Q_OS_MACOS)
    path = QCoreApplication::applicationDirPath() + "/../Resources/resource.brc";

#if 0
    QString command { QDir::homePath() + "/Qt6.5.3/6.5.3/macos/libexec/rcc" + " -binary " + QDir::homePath() + "/Documents/YtxCash/resource/resource.qrc -o "
        + path };

    QProcess process {};
    process.start("zsh", QStringList() << "-c" << command);
    process.waitForFinished();
#endif

#endif

    QResource::registerResource(path);
}

void MainWindow::RSearchTriggered()
{
    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    auto dialog { new Search(&section_data_->tree_info, section_tree_->model, &section_data_->table_info, &section_data_->search_sql, section_rule_,
        &section_data_->tree_info.unit_hash, &node_rule_hash_, &section_data_->tree_info.unit_symbol_hash, this) };

    connect(dialog, &Search::SSearchedNode, this, &MainWindow::RSearchedNode, Qt::UniqueConnection);
    connect(dialog, &Search::SSearchedTrans, this, &MainWindow::RSearchedTrans, Qt::UniqueConnection);
    connect(section_tree_->model, &TreeModel::SSearch, dialog, &Search::RSearch, Qt::UniqueConnection);

    dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    section_dialog_->append(dialog);
    dialog->show();
}

void MainWindow::RSearchedNode(int node_id)
{
    auto view { section_tree_->view };
    auto model { section_tree_->model };
    ui->tabWidget->setCurrentWidget(section_tree_->view);
    view->activateWindow();

    auto index { model->GetIndex(node_id) };
    view->setCurrentIndex(index);
}

void MainWindow::RSearchedTrans(int trans_id, int lhs_node_id, int rhs_node_id)
{
    auto Switch = [&](int node_id) {
        if (section_view_->contains(node_id)) {
            SwitchTab(node_id);
            return true;
        }
        return false;
    };

    if (!Switch(lhs_node_id) && !Switch(rhs_node_id)) {
        auto node = section_tree_->model->GetNode(lhs_node_id);
        CreateTable(node->name, node->id, node->node_rule);
    }

    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTableView(widget))
        return;

    auto view { GetTableView(widget) };
    auto model { GetTableModel(view) };

    int row { model->TransRow(trans_id) };
    if (row == -1)
        return;
    auto index { model->index(row, 0, QModelIndex()) };

    view->activateWindow();
    view->setCurrentIndex(index);
}

void MainWindow::RPreferencesTriggered()
{
    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    auto model { section_tree_->model };
    auto leaf_path { model->LeafPath() };
    auto branch_path { model->BranchPath() };

    auto preference { new Preferences(&section_data_->tree_info.unit_hash, leaf_path, branch_path, &date_format_list_, interface_, *section_rule_, this) };
    connect(preference, &Preferences::SUpdateSettings, this, &MainWindow::RUpdateSettings, Qt::UniqueConnection);
    preference->exec();
}

void MainWindow::RAboutTriggered()
{
    static About* dialog = nullptr;

    if (!dialog) {
        dialog = new About(this);
        dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &QDialog::finished, [=]() { dialog = nullptr; });
    }

    dialog->show();
    dialog->activateWindow();
}

void MainWindow::RNewTriggered()
{
    QString filter("*.ytxcash");
    auto file_path { QFileDialog::getSaveFileName(this, tr("New File"), QDir::homePath(), filter, nullptr) };
    if (file_path.isEmpty())
        return;

    if (!file_path.endsWith(".ytxcash", Qt::CaseInsensitive))
        file_path += ".ytxcash";

    sql_.NewFile(file_path);
    ROpenFile(file_path);
}

void MainWindow::ROpenTriggered()
{
    QString filter("*.ytxcash");
    auto file_path { QFileDialog::getOpenFileName(this, tr("Select File"), QDir::homePath(), filter, nullptr) };

    ROpenFile(file_path);
}

void MainWindow::SetStatusBar()
{
    status_bar_.static_label = new QLabel(this);
    status_bar_.static_spin = new QDoubleSpinBox(this);
    status_bar_.dynamic_label = new QLabel(this);
    status_bar_.dynamic_spin = new QDoubleSpinBox(this);

    status_bar_.static_spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    status_bar_.static_spin->setRange(-1000000000, 1000000000);
    status_bar_.static_spin->setReadOnly(true);
    status_bar_.static_spin->setGroupSeparatorShown(true);

    status_bar_.dynamic_spin->setRange(-1000000000, 1000000000);
    status_bar_.dynamic_spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    status_bar_.dynamic_spin->setReadOnly(true);
    status_bar_.dynamic_spin->setGroupSeparatorShown(true);

    auto space_before_middle { new QWidget(this) };
    space_before_middle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto space_after_middle { new QWidget(this) };
    space_after_middle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    ui->statusbar->addPermanentWidget(space_before_middle);
    ui->statusbar->addPermanentWidget(status_bar_.static_label);
    ui->statusbar->addPermanentWidget(status_bar_.static_spin);
    ui->statusbar->addPermanentWidget(space_after_middle);
    ui->statusbar->addPermanentWidget(status_bar_.dynamic_label);
    ui->statusbar->addPermanentWidget(status_bar_.dynamic_spin);

    ui->statusbar->hide();
}

void MainWindow::SetClearMenuAction()
{
    auto menu { ui->menuRecent };

    if (!menu->isEmpty()) {
        auto separator { ui->actionSeparator };
        menu->addAction(separator);
        separator->setSeparator(true);

        menu->addAction(ui->actionClearMenu);
    }
}

template <typename T>
    requires InheritQWidget<T>
void MainWindow::SaveState(T* widget, QSettings* interface, CString& section_name, CString& property)
{
    auto state { widget->saveState() };
    interface->setValue(QString("%1/%2").arg(section_name, property), state);
}

template <typename T>
    requires InheritQWidget<T>
void MainWindow::RestoreState(T* widget, QSettings* interface, CString& section_name, CString& property)
{
    auto state { interface->value(QString("%1/%2").arg(section_name, property)).toByteArray() };

    if (!state.isEmpty())
        widget->restoreState(state);
}

template <typename T>
    requires InheritQWidget<T>
void MainWindow::SaveGeometry(T* widget, QSettings* interface, CString& section_name, CString& property)
{
    auto geometry { widget->saveGeometry() };
    interface->setValue(QString("%1/%2").arg(section_name, property), geometry);
}

template <typename T>
    requires InheritQWidget<T>
void MainWindow::RestoreGeometry(T* widget, QSettings* interface, CString& section_name, CString& property)
{
    auto geometry { interface->value(QString("%1/%2").arg(section_name, property)).toByteArray() };
    if (!geometry.isEmpty())
        widget->restoreGeometry(geometry);
}

void MainWindow::RClearMenuTriggered()
{
    ui->menuRecent->clear();
    recent_.clear();
    UpdateRecent();
}

void MainWindow::RTabBarDoubleClicked(int index)
{
    int node_id { ui->tabWidget->tabBar()->tabData(index).value<Tab>().node };
    RSearchedNode(node_id);
}

void MainWindow::SwitchSection(CString& name)
{
    auto tab_widget { ui->tabWidget };
    auto tab_bar { ui->tabWidget->tabBar() };
    int count { ui->tabWidget->count() };

    if (!section_tree_->view) {
        for (int index = 0; index != count; ++index)
            tab_widget->setTabVisible(index, false);

        *section_tree_ = CreateTree(&section_data_->tree_info, &section_data_->tree_sql, section_rule_, &interface_);
        auto view { section_tree_->view };

        CreateDelegate(view);
        SetConnect(view, section_tree_->model);

        auto node { section_data_->tree_info.node };
        int tab_index { tab_widget->addTab(view, name) };
        Tab tab_id { start_, 0 };
        tab_bar->setTabData(tab_index, QVariant::fromValue(tab_id));

        RestoreView(node, "view");
        RestoreState(view->header(), dynamic_interface_, node, "header_state");

        SetView(view);
    } else {
        for (int index = 0; index != count; ++index) {
            if (tab_bar->tabData(index).value<Tab>().section == start_)
                tab_widget->setTabVisible(index, true);
            else
                tab_widget->setTabVisible(index, false);
        }

        if (section_dialog_) {
            for (auto& dialog : *section_dialog_) {
                if (dialog)
                    dialog->setEnabled(true);
            }
        }
    }

    tab_widget->setCurrentWidget(section_tree_->view);
    UpdateStatusBar();
}

void MainWindow::RUpdateState()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTableView(widget))
        return;

    auto view { GetTableView(widget) };
    auto model { GetTableModel(view) };

    auto sender_check { Check(QObject::sender()->property("check").toInt()) };
    model->UpdateState(sender_check);
}

void MainWindow::RFinanceToggled(bool checked)
{
    start_ = Section::kFinance;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    if (checked) {
        if (section_dialog_) {
            for (auto& dialog : *section_dialog_) {
                if (dialog)
                    dialog->setEnabled(false);
            }
        }

        section_tree_ = &finance_tree_;
        section_view_ = &finance_view_;
        section_dialog_ = &finance_dialog_;
        section_rule_ = &finance_rule_;
        section_data_ = &finance_data_;

        static auto name = tr("Finance");
        SwitchSection(name);
    }
}

void MainWindow::RProductToggled(bool checked)
{
    start_ = Section::kProduct;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    if (checked) {
        if (section_dialog_) {
            for (auto& dialog : *section_dialog_) {
                if (dialog)
                    dialog->setEnabled(false);
            }
        }

        section_tree_ = &product_tree_;
        section_view_ = &product_view_;
        section_dialog_ = &product_dialog_;
        section_rule_ = &product_rule_;
        section_data_ = &product_data_;

        static auto name = tr("Product");
        SwitchSection(name);
    }
}

void MainWindow::RNetworkToggled(bool checked)
{
    start_ = Section::kNetwork;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    if (checked) {
        if (section_dialog_) {
            for (auto& dialog : *section_dialog_) {
                if (dialog)
                    dialog->setEnabled(false);
            }
        }

        section_tree_ = &network_tree_;
        section_view_ = &network_view_;
        section_dialog_ = &network_dialog_;
        section_rule_ = &network_rule_;
        section_data_ = &network_data_;

        static auto name = tr("Network");
        SwitchSection(name);
    }
}

void MainWindow::RBtnTaskToggled(bool checked)
{
    start_ = Section::kTask;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    if (checked) {
        if (section_dialog_) {
            for (auto& dialog : *section_dialog_) {
                if (dialog)
                    dialog->setEnabled(false);
            }
        }

        section_tree_ = &task_tree_;
        section_view_ = &task_view_;
        section_dialog_ = &task_dialog_;
        section_rule_ = &task_rule_;
        section_data_ = &task_data_;

        static auto name = tr("Task");
        SwitchSection(name);
    }
}
