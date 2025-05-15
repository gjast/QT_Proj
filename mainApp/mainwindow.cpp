#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <tradingbotapiclient.h>
#include <QDebug>
#include <QLocale>

MainWindow::MainWindow(const QString &serverUrl, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_apiClient(new TradingBotAPIClient(serverUrl, this))
{
    ui->setupUi(this);
    setWindowTitle("Crypto Trading Platform");
    resize(500, 400);
    connect(ui->on_sellAllButton_clicked, &QPushButton::clicked, this, &MainWindow::on_sellAllButton_clicked);

    // Установка стилей для кнопок
    ui->on_confirmButton_clicked->setStyleSheet("background-color: #4CAF50; color: white;");
    ui->on_buyButton_clicked->setStyleSheet("background-color: #2ECC71; color: white;");
    ui->on_sellButton_clicked->setStyleSheet("background-color: #E74C3C; color: white;");
    ui->on_sellAllButton_clicked->setStyleSheet("background-color: #E74C3C; color: white;");

    // Подключение сигнала к слоту
    connect(m_apiClient, &TradingBotAPIClient::apiResponseReceived,
            this, &MainWindow::handleApiResponse);

    connect(m_apiClient, &TradingBotAPIClient::apiResponseReceived,
            [](const QJsonObject &response) {
                qDebug() << "Получен ответ от сервера:";
                qDebug() << QJsonDocument(response).toJson(QJsonDocument::Indented);
            });

    updateMarketData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateMarketData()
{
    ui->priceLabel->setText("current price: -");
    ui->resistanceLabel->setText("resistance level: -");
    ui->supportLabel->setText("support level: -");
    ui->rsiLabel->setText("RSI: -");
    ui->macdLabel->setText("MACD: -");
    ui->forecastLabel->setText("Forecast: -");
}

void MainWindow::on_confirmButton_clicked()
{
    m_pair = ui->pairEdit->text().trimmed().toUpper();
    m_price = ui->priceEdit->text().trimmed();
    m_takeProfit = ui->takeProfit->text().trimmed();
    m_stopLoss = ui->stopLoss->text().trimmed();

    if(m_pair.isEmpty() || m_price.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill all fields");
        return;
    }

    bool ok;
    double priceValue = m_price.toDouble(&ok);

    if (!ok) {
        QMessageBox::warning(this, "Error", "Invalid price format");
        return;
    }

    QMessageBox::information(this, "Confirmed",
                             QString("Pair: %1\nPrice: %2").arg(m_pair).arg(m_price));

    // Сохраняем текущую пару для идентификации ответов
    m_currentPair = m_pair;

    // Отправляем запросы
    m_apiClient->getMarketData(m_pair);
    m_apiClient->getPrice(m_pair);
}
void MainWindow::handleApiResponse(const QJsonObject &response)
{
    if (response.isEmpty()) {
        qDebug() << "Empty API response received";
        return;
    }

    qDebug() << "Full API response:" << QJsonDocument(response).toJson(QJsonDocument::Indented);

    // Определяем тип ответа по содержимому
    if (response.contains("rsi") || response.contains("resistance") || response.contains("support")) {
        handleMarketDataResponse(response);
    }
    else if (response.contains("price")) {
        handlePriceResponse(response);
    }
    else {
        qDebug() << "Unknown response type";
        qDebug() << "Available keys:" << response.keys();
    }
}

void MainWindow::handleMarketDataResponse(const QJsonObject &response)
{
    try {
        // Обрабатываем RSI
        if (response.contains("rsi")) {
            double rsiValue = response["rsi"].toDouble();
            ui->rsiLabel->setText(QString("RSI: %1").arg(rsiValue, 0, 'f', 2));
        } else {
            ui->rsiLabel->setText("RSI: N/A");
        }

        // Обрабатываем уровень сопротивления
        if (response.contains("resistance")) {
            double resistance = response["resistance"].toDouble();
            ui->resistanceLabel->setText(QString("Resistance: %1").arg(resistance, 0, 'f', 2));
        } else {
            ui->resistanceLabel->setText("Resistance: N/A");
        }

        // Обрабатываем уровень поддержки
        if (response.contains("support")) {
            double support = response["support"].toDouble();
            ui->supportLabel->setText(QString("Support: %1").arg(support, 0, 'f', 2));
        } else {
            ui->supportLabel->setText("Support: N/A");
        }

        // Обрабатываем прогноз
        if (response.contains("shouldBuy")) {
            QString forecast = response["shouldBuy"].toBool() ? "Buy" : "Sell";
            ui->forecastLabel->setText(QString("Forecast: %1").arg(forecast));
        } else {
            ui->forecastLabel->setText("Forecast: N/A");
        }

    } catch (const std::exception &e) {
        qDebug() << "Error processing market data:" << e.what();
        QMessageBox::warning(this, "Error", "Failed to process market data");
    }
}

void MainWindow::handlePriceResponse(const QJsonObject &response)
{
    try {
        if (response.contains("price")) {
            QJsonValue priceValue = response["price"];

            // Проверяем разные форматы цены
            double price = 0;
            if (priceValue.isDouble()) {
                price = priceValue.toDouble();
            }
            else if (priceValue.isString()) {
                bool ok;
                price = priceValue.toString().toDouble(&ok);
                if (!ok) {
                    qDebug() << "Failed to convert price string to double";
                    ui->priceLabel->setText("Price: Invalid format");
                    return;
                }
            }

            // Форматируем цену с 4 знаками после запятой
            QString formattedPrice = QLocale().toString(price, 'f', 2);
            ui->priceLabel->setText(QString("Current price: %1").arg(formattedPrice));

            qDebug() << "Price successfully set to:" << formattedPrice;
        } else {
            ui->priceLabel->setText("Price: N/A");
            qDebug() << "Price field missing in response";
        }
    } catch (const std::exception &e) {
        qDebug() << "Error processing price data:" << e.what();
        QMessageBox::warning(this, "Error", "Failed to process price data");
        ui->priceLabel->setText("Price: Error");
    }
}

void MainWindow::on_buyButton_clicked()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Buy Order");
    msgBox.setText("Buy order placed!");
    msgBox.setModal(true);
    msgBox.exec();
    m_apiClient->placeLeveragedOrder(m_pair, "Buy", m_price, m_takeProfit, m_stopLoss);
}

void MainWindow::on_sellButton_clicked()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Sell Order");
    msgBox.setText("Sell order placed!");
    msgBox.setModal(true);
    msgBox.exec();
    m_apiClient->placeLeveragedOrder(m_pair, "Sell", m_price, m_takeProfit, m_stopLoss);
}

void MainWindow::on_sellAllButton_clicked()
{
    m_apiClient->cancelAllOrders();
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Sell Order");
    msgBox.setText("Sell order placed!");
    msgBox.setModal(true);
    msgBox.exec();
}

