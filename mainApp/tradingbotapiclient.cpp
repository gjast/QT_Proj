#include "tradingbotapiclient.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QJsonDocument>

TradingBotAPIClient::TradingBotAPIClient(const QString &baseUrl, QObject *parent)
    : QObject(parent), m_baseUrl(baseUrl), m_apiKey(""), m_apiSecret("")
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &TradingBotAPIClient::handleNetworkReply);
}

void TradingBotAPIClient::setApiKeys(const QString &apiKey, const QString &apiSecret)
{
    m_apiKey = apiKey;
    m_apiSecret = apiSecret;

    QJsonObject keysData;
    keysData["apiKey"] = apiKey;
    keysData["apiSecret"] = apiSecret;

    sendPostRequest("/api/set-keys", keysData);
}

void TradingBotAPIClient::startBot(const QString &symbol)
{
    QJsonObject jsonParams;
    jsonParams["symbol"] = symbol;

    sendPostRequest("/start-bot", jsonParams);

}

void TradingBotAPIClient::getMarketData(const QString &symbol, const QString &orderType)
{
    QString endpoint = QString("/market-data/%1?orderType=%2").arg(symbol, orderType);
    sendGetRequest(endpoint);
}

void TradingBotAPIClient::getBalance()
{
    sendGetRequest("/balance");
}

void TradingBotAPIClient::getPrice(const QString &symbol, const QString &orderType)
{
    QString endpoint = QString("/price/%1?orderType=%2").arg(symbol, orderType);
    sendGetRequest(endpoint);
}

void TradingBotAPIClient::cancelAllOrders()
{
    sendPostRequest("/cancel-orders", QJsonObject());
}

void TradingBotAPIClient::placeLeveragedOrder(const QString &symbol, const QString &side, const QString &quantity,
                                              const QString &takeProfit, const QString &stopLoss)
{
    QJsonObject orderData;
    orderData["symbol"] = symbol;
    orderData["side"] = side;
    orderData["quantity"] = quantity;
    orderData["takeProfit"] = takeProfit;
    orderData["stopLoss"] = stopLoss;

    sendPostRequest("/place-leveraged-order", orderData);
}

void TradingBotAPIClient::sendPostRequest(const QString &endpoint, const QJsonObject &data)
{
    QUrl url(m_baseUrl + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(data);
    QByteArray postData = doc.toJson();

    m_networkManager->post(request, postData);
}

void TradingBotAPIClient::sendGetRequest(const QString &endpoint)
{
    QUrl url(m_baseUrl + endpoint);
    QNetworkRequest request(url);

    m_networkManager->get(request);
}

void TradingBotAPIClient::handleNetworkReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject responseObject = jsonResponse.object();

    emit apiResponseReceived(responseObject);

    reply->deleteLater();
}
