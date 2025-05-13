#include "authdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AuthDialog authDialog;
    if (authDialog.exec() == QDialog::Accepted) {
        return a.exec();
    }

    return 0;
}
