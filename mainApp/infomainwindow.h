#ifndef INFOMAINWINDOW_H
#define INFOMAINWINDOW_H

#include <QDialog>
#include "qcustomplot.h" // Добавляем заголовочный файл QCustomPlot
#include <QLabel>
namespace Ui {
class InfoMainWindow;
}

class InfoMainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit InfoMainWindow(QWidget *parent = nullptr);
    ~InfoMainWindow();
    void setFooterText(const QString& text);
    void setMarketData(const QList<QList<QString>>& candlesData,
                       double support,
                       double resistance);
    void setInfo(const QString& title, const QString& message);

private:
    Ui::InfoMainWindow *ui;
    QCustomPlot *customPlot;
    QLabel *footerLabel;    // Добавляем указатель на QCustomPlot
    void setupCandlestickChart(); // Метод для настройки графика
};

#endif // INFOMAINWINDOW_H
