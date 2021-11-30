#include "newdialog.hh"

#include "QColorDialog"
#include "moc_newdialog.cpp"
#include "ui_newdialog.h"

NewDialog::NewDialog(QWidget *parent) : QDialog(parent), ui(new Ui::NewDialog) {
  ui->setupUi(this);
}

NewDialog::~NewDialog() { delete ui; }
