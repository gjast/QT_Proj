#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_confirmButton_clicked();
    void on_buyButton_clicked();
    void on_sellButton_clicked();

private:
    Ui::MainWindow *ui;
    void updateMarketData();
};
#endif // MAINWINDOW_H
