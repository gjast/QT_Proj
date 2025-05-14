#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QDialog>
#include "tradingbotapiclient.h"

namespace Ui {
class AuthDialog;
}

class AuthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthDialog(const QString &serverUrl, QWidget *parent = nullptr);
    ~AuthDialog();

private slots:
    void onLoginClicked();
    void handleApiResponse(const QJsonObject &response);
    void handleError(const QString &error);

private:
    Ui::AuthDialog *ui;
    TradingBotAPIClient *m_apiClient;
};

#endif // AUTHDIALOG_H
