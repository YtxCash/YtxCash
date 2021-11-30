#ifndef PREFERENCESDIALOG_HH
#define PREFERENCESDIALOG_HH

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog {
  Q_OBJECT

 public:
  explicit PreferencesDialog(QWidget *parent = nullptr);
  ~PreferencesDialog();

 private slots:

 private:
  Ui::PreferencesDialog *ui;
};

#endif  // PREFERENCESDIALOG_HH
