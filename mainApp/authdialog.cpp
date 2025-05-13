#include "authdialog.h"
#include "ui_authdialog.h"
#include "mainwindow.h"
#include <QMessageBox>

AuthDialog::AuthDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthDialog)
{
    ui->setupUi(this);
    setWindowTitle("Авторизация");
}

AuthDialog::~AuthDialog()
{
    delete ui;
}

void AuthDialog::onLoginClicked()
{
    QString login = ui->loginEdit->text();
    QString password = ui->passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все поля");
        return;
    }

    // Закрываем диалог авторизации
    this->accept();

    // Открываем главное окно
    MainWindow *mainWindow = new MainWindow(); // Создаем экземпляр MainWindow
    mainWindow->show(); // Показываем окно
}
