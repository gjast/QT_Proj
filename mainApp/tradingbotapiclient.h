#ifndef TRADINGBOTAPICLIENT_H
#define TRADINGBOTAPICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

class TradingBotAPIClient : public QObject
{
    Q_OBJECT
public:
    explicit TradingBotAPIClient(const QString &baseUrl, QObject *parent = nullptr);

    void setApiKeys(const QString &apiKey, const QString &apiSecret);
    void startBot();
    void getMarketData(const QString &symbol, const QString &orderType = "linear");
    void getBalance();
    void getPrice(const QString &symbol, const QString &orderType = "linear");
    void cancelAllOrders();
    void placeLeveragedOrder(const QString &symbol, const QString &side, const QString &quantity,
                             const QString &takeProfit, const QString &stopLoss);

signals:
    void apiResponseReceived(const QJsonObject &response);
    void errorOccurred(const QString &error);

private slots:
    void handleNetworkReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_baseUrl;
    QString m_apiKey;
    QString m_apiSecret;

    void sendPostRequest(const QString &endpoint, const QJsonObject &data);
    void sendGetRequest(const QString &endpoint);
};

#endif // TRADINGBOTAPICLIENT_H
