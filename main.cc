#include <QApplication>
#include <QLocale>
#include <QStyleFactory>
#include <QTranslator>

#include "mainwindow.hh"

int main(int argc, char *argv[]) {
  QApplication ytx_cash(argc, argv);
  ytx_cash.setStyle(QStyleFactory::create("fusion"));

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString &locale : uiLanguages) {
    const QString baseName = "YtxCash_" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      ytx_cash.installTranslator(&translator);
      break;
    }
  }
  MainWindow window;
  window.setWindowTitle("YtxCash");
  window.show();

  return ytx_cash.exec();
}
