#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class InfoWindow; }
QT_END_NAMESPACE

class InfoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit InfoWindow(QWidget *parent = nullptr);
    ~InfoWindow();

private:
    Ui::InfoWindow *ui;  // Указатель на UI-класс
};

#endif // INFOWINDOW_H
