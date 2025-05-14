#include "authdialog.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString serverUrl = "http://localhost:3000"; // или ваш реальный URL

    AuthDialog authDialog(serverUrl);
    if (authDialog.exec() == QDialog::Accepted) {
        MainWindow mainWindow(serverUrl);
        mainWindow.show();
        return a.exec();
    }

    return 0;
}
