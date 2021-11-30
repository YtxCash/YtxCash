#include "preferencesdialog.hh"

#include "moc_preferencesdialog.cpp"
#include "ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::PreferencesDialog) {
  ui->setupUi(this);
}

PreferencesDialog::~PreferencesDialog() { delete ui; }
