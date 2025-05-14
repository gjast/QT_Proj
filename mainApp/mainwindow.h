#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <tradingbotapiclient.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &serverUrl, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_confirmButton_clicked();
    void on_buyButton_clicked();
    void on_sellButton_clicked();
    void on_sellAllButton_clicked();
    void handleApiResponse(const QJsonObject &response);

private:
    void updateMarketData();
    void handleMarketDataResponse(const QJsonObject &response);
    void handlePriceResponse(const QJsonObject &response);

private:
    QString m_pair;  // Текущая выбранная пара
    QString m_price; // Текущая цена
    QString m_takeProfit; // Take Profit значение
    QString m_stopLoss;   // Stop Loss значение

    Ui::MainWindow *ui; // Указатель на интерфейс пользователя
    TradingBotAPIClient *m_apiClient; // Указатель на API клиент
    QString m_currentPair; // Текущая пара
};



#endif // MAINWINDOW_H
