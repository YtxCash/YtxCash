#ifndef NEWDIALOG_HH
#define NEWDIALOG_HH

#include <QDialog>

namespace Ui {
class NewDialog;
}

class NewDialog : public QDialog {
  Q_OBJECT

 public:
  explicit NewDialog(QWidget *parent = nullptr);
  ~NewDialog();

 private:
  Ui::NewDialog *ui;
};

#endif  // NEWDIALOG_HH
