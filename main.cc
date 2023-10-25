#include <QApplication>
#include <QDir>
#include <QLockFile>

#include "mainwindow.h"

#ifdef Q_OS_MACOS
#include "component/app.h"
#endif

int main(int argc, char* argv[])
{
// begin set ini file directory
#ifdef Q_OS_WIN
    QApplication app(argc, argv);
    QString path { QDir::homePath() + "/AppData/Roaming/YtxCash" };
#elif defined(Q_OS_MACOS)
    App app(argc, argv);
    QString path { QDir::homePath() + "/.config/YtxCash" };
#endif

    if (QDir dir { path }; !dir.exists()) {
        if (!QDir::home().mkpath(path)) {
            qDebug() << "Failed to create directory:" << path;
            return 1;
        }
    }
    // end set ini file directory

    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);

    // begin set database file, then try to lock it, if false return
    QString file_path {};

    if (app.arguments().size() >= 2) {
#ifdef Q_OS_WIN
        file_path = QString::fromLocal8Bit(argv[1]);
#elif defined(Q_OS_MACOS)
        file_path = app.arguments().at(1);
#endif
    }

    if (!file_path.isEmpty()) {
        QFileInfo file_info(file_path);
        auto lock_file_path { file_info.absolutePath() + "/" + file_info.baseName() + ".lock" };

        static QLockFile lock_file { lock_file_path };
        if (!lock_file.tryLock(100))
            return 1;
    }
    // end set database file

    MainWindow w(path);
    w.show();

    if (!file_path.isEmpty())
        w.ROpenFile(file_path);

#ifdef Q_OS_MACOS
    QObject::connect(&app, &App::SOpenFile, &w, &MainWindow::ROpenFile, Qt::UniqueConnection);
#endif

    return app.exec();
}
