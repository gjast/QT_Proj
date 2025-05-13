#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Crypto Trading Platform");
    resize(500, 400);

    // Установка стилей
    ui->on_confirmButton_clicked->setStyleSheet("background-color: #4CAF50; color: white;");
    ui->buyButton->setStyleSheet("background-color: #2ECC71; color: white;");
    ui->sellButton->setStyleSheet("background-color: #E74C3C; color: white;");

    updateMarketData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateMarketData()
{
    // Здесь можно добавить обновление данных с API
    ui->priceLabel->setText("current price: 101000");
    ui->resistanceLabel->setText("resistance level: 105000");
    ui->supportLabel->setText("support level: 95000");
    ui->rsiLabel->setText("RSI: 33");
    ui->macdLabel->setText("MACD: 49");
}

void MainWindow::on_confirmButton_clicked()
{
    QString pair = ui->pairEdit->text();
    QString price = ui->priceEdit->text();

    if(pair.isEmpty() || price.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill all fields");
        return;
    }

    QMessageBox::information(this, "Confirmed",
                             QString("Pair: %1\nPrice: %2").arg(pair).arg(price));
}

void MainWindow::on_buyButton_clicked()
{
    QMessageBox::information(this, "Buy Order", "Buy order placed!");
}

void MainWindow::on_sellButton_clicked()
{
    QMessageBox::information(this, "Sell Order", "Sell order placed!");
}
