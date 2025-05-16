#include "infowindow.h"
#include "ui_infowindow.h"  // Важно: имя должно совпадать с UI-файлом

InfoWindow::InfoWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::InfoWindow)
{
    ui->setupUi(this);
    setWindowTitle("Info Window");
    resize(400, 300);
}

InfoWindow::~InfoWindow()
{
    delete ui;
}
