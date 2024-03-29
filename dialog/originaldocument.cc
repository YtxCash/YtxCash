#include "originaldocument.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

#include "ui_originaldocument.h"

OriginalDocument::OriginalDocument(Section section, SPTrans& trans, CString& document_dir, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::OriginalDocument)
    , section { section }
    , trans_ { trans }
{
    ui->setupUi(this);
    CreateList(trans->document);
    IniDialog();
    IniConnect();
    document_dir_ = QDir::homePath() + "/" + document_dir;
}

OriginalDocument::~OriginalDocument()
{
    delete list_model_;
    delete ui;
}

void OriginalDocument::on_pBtnAdd_clicked()
{
    QString filter("*.pdf");
    auto local_documents { QFileDialog::getOpenFileNames(this, tr("Select Original Document"), document_dir_, filter, nullptr) };

    if (local_documents.isEmpty())
        return;

    auto existing_documents { trans_->document };
    int row {};

    for (const QString& document : local_documents) {
        if (!existing_documents.contains(document)) {
            row = list_model_->rowCount();
            list_model_->insertRow(row);
            list_model_->setData(list_model_->index(row), QDir::home().relativeFilePath(document));
        }
    }
}

void OriginalDocument::on_pBtnRemove_clicked()
{
    auto index { ui->listView->currentIndex() };
    list_model_->removeRow(index.row(), QModelIndex());
}

void OriginalDocument::RCustomAccept()
{
    trans_->document = list_model_->stringList();
    emit SUpdateTrans(section, trans_, TableColumn::kDocument);
    accept();
}

void OriginalDocument::on_listView_doubleClicked(const QModelIndex& index)
{
    QString file_path { QDir::homePath() + "/" + index.data().toString() };
    auto file_url { QUrl::fromLocalFile(file_path) };

    if (QFile::exists(file_path)) {
        QDesktopServices::openUrl(file_url);
    } else {
        QMessageBox msg;
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Doesn't Exit"));
        msg.setInformativeText(tr("File doesn't exit, please check again."));
        msg.exec();
    }
}

void OriginalDocument::CreateList(CStringList& list)
{
    list_model_ = new QStringListModel(this);
    list_model_->setStringList(list);
    ui->listView->setModel(list_model_);
}

void OriginalDocument::IniDialog()
{
    ui->pBtnOk->setDefault(true);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setWindowTitle(tr("Original Document"));

    QSize mainwindow_size = qApp->activeWindow()->size();
    int width = mainwindow_size.width() * 720 / 1920;
    int height = mainwindow_size.height() * 540 / 1080;
    this->resize(width, height);
}

void OriginalDocument::IniConnect()
{
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &QDialog::reject, Qt::UniqueConnection);
    connect(ui->pBtnOk, &QPushButton::clicked, this, &OriginalDocument::RCustomAccept, Qt::UniqueConnection);
}
