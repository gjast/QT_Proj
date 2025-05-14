#include "authdialog.h"
#include "ui_authdialog.h"
#include <QMessageBox>

AuthDialog::AuthDialog(const QString &serverUrl, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthDialog),
    m_apiClient(new TradingBotAPIClient(serverUrl, this))
{
    ui->setupUi(this);
    setWindowTitle("Авторизация");

    // Подключаем кнопку
    connect(ui->loginButton, &QPushButton::clicked, this, &AuthDialog::onLoginClicked);

    // Подключаем сигналы от API клиента
    connect(m_apiClient, &TradingBotAPIClient::apiResponseReceived,
            this, &AuthDialog::handleApiResponse);
    connect(m_apiClient, &TradingBotAPIClient::errorOccurred,
            this, &AuthDialog::handleError);
}

AuthDialog::~AuthDialog()
{
    delete ui;
}

void AuthDialog::onLoginClicked()
{
    QString apiKey = ui->loginEdit->text();
    QString apiSecret = ui->passwordEdit->text();

    if (apiKey.isEmpty() || apiSecret.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все поля");
        return;
    }

    // Отправляем ключи на сервер
    m_apiClient->setApiKeys(apiKey, apiSecret);

    // Показываем индикатор загрузки
    ui->loginButton->setEnabled(false);
    ui->loginButton->setText("Отправка...");
}

void AuthDialog::handleApiResponse(const QJsonObject &response)
{
    // Проверяем, это ответ на установку ключей
    if (response.contains("success") && response["success"].toBool()) {
        QMessageBox::information(this, "Успех", "Вы вошли");

        // Восстанавливаем кнопку перед закрытием
        ui->loginButton->setEnabled(true);
        ui->loginButton->setText("Войти");

        // Закрываем диалог с результатом Accepted
        this->accept();
    } else {
        // Восстанавливаем кнопку при неудаче
        ui->loginButton->setEnabled(true);
        ui->loginButton->setText("Войти");
    }
}

void AuthDialog::handleError(const QString &error)
{
    QMessageBox::critical(this, "Ошибка", "Вы не вошли: " + error);

    // Восстанавливаем кнопку
    ui->loginButton->setEnabled(true);
    ui->loginButton->setText("Войти");

    // Очищаем поля
    ui->passwordEdit->clear();
}
