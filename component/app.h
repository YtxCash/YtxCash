#ifndef APP_H
#define APP_H

#include <QApplication>
#include <QFileOpenEvent>

class App : public QApplication {
    Q_OBJECT

signals:
    void SOpenFile(const QString& file_path);

public:
    App(int& argc, char** argv)
        : QApplication(argc, argv)
    {
    }

protected:
    bool event(QEvent* event) override
    {
        if (event->type() == QEvent::FileOpen) {
            auto open_event { static_cast<QFileOpenEvent*>(event) };
            emit SOpenFile(open_event->file());
        }

        return QApplication::event(event);
    }
};

#endif // APP_H
