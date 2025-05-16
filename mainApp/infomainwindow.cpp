#include "infomainwindow.h"
#include "ui_infomainwindow.h"

InfoMainWindow::InfoMainWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoMainWindow)
{
    ui->setupUi(this);

    // Базовые настройки окна
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(700, 600);

    // Создаем и настраиваем QCustomPlot
    customPlot = new QCustomPlot(this);
    ui->verticalLayout->addWidget(customPlot);

    // Создаем QScrollArea для прокрутки текста
    QScrollArea *scrollArea = new QScrollArea(this);
    int textHeight = this->height() * 0.3; // 30% от высоты окна
    scrollArea->setMinimumHeight(textHeight);
    scrollArea->setMaximumHeight(textHeight);

    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Создаем QLabel для текста
    footerLabel = new QLabel();
    footerLabel->setWordWrap(true);
    footerLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    footerLabel->setStyleSheet("QLabel { color: gray; font-size: 10pt; padding: 5px; }");
    footerLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // Помещаем QLabel в QScrollArea
    scrollArea->setWidget(footerLabel);

    // Добавляем QScrollArea в layout
    ui->verticalLayout->addWidget(scrollArea);


    setFooterText(
        "📊 Как читать график:\n\n"

        "🔵 Уровни поддержки и сопротивления:\n"
        "   - Поддержка (нижняя линия) - цена как бы 'отталкивается' от этой зоны вверх\n"
        "   - Сопротивление (верхняя линия) - цена 'спотыкается' об эту зону и падает\n"
        "   Пример: Если цена 3 раза не могла пробить $50 - это сильное сопротивление\n\n"

        "📈 MACD - индикатор тренда и моментума:\n"
        "   1. Синяя линия (MACD): Разница между 12-дневной и 26-дневной скользящими средними\n"
        "      Формула: MACD = EMA(12) - EMA(26)\n"
        "   2. Оранжевая линия (Signal): Сглаженная версия синей линии\n"
        "      Формула: Signal = EMA(MACD за 9 дней)\n"
        "   3. Гистограмма: Разница между синей и оранжевой линиями\n"
        "      Чем выше столбики - тем сильнее тренд\n\n"

        "📊 RSI - индикатор перекупленности/перепроданности:\n"
        "   - Показывает насколько актив 'перегрет'\n"
        "   Формула: RSI = 100 - (100 / (1 + средний рост / среднее падение за 14 дней))\n"
        "   • Выше 70 - перекупленность (возможен спад)\n"
        "   • Ниже 30 - перепроданность (возможен рост)\n\n"

        "🔮 Как предсказать движение цены:\n"
        "▲ ВВЕРХ если:\n"
        "   - MACD синяя линия ПЕРЕСЕКАЕТ оранжевую СНИЗУ ВВЕРХ\n"
        "   - RSI выше 30 (но ниже 70 - идеально 40-60)\n"
        "   - Цена пробивает уровень сопротивления\n\n"

        "▼ ВНИЗ если:\n"
        "   - MACD синяя линия ПЕРЕСЕКАЕТ оранжевую СВЕРХУ ВНИЗ\n"
        "   - RSI ниже 70 (но выше 30 - идеально 40-60)\n"
        "   - Цена пробивает уровень поддержки\n\n"

        "💡 Совет: Всегда проверяйте несколько индикаторов сразу!"
        );
}

void InfoMainWindow::setFooterText(const QString &text)
{
    if(footerLabel) {
        footerLabel->setText(text);
        // Обновляем размеры после установки текста
        footerLabel->adjustSize();
    }
}

InfoMainWindow::~InfoMainWindow()
{
    delete ui;
}

void InfoMainWindow::setInfo(const QString& title, const QString& message)
{
    setWindowTitle(title);
}


void InfoMainWindow::setMarketData(const QList<QList<QString>>& candlesData,
                                   double support,
                                   double resistance)
{
    // Вывод информации в консоль
    qDebug() << "=== 4-Hour Candles Data ===";
    qDebug() << "Support level:" << support;
    qDebug() << "Resistance level:" << resistance;
    qDebug() << "Total 4-hour candles received:" << candlesData.size();

    // Подготовка данных для графика
    QVector<QCPFinancialData> financialData;
    QVector<double> timeKeys;
    double minPrice = std::numeric_limits<double>::max();
    double maxPrice = std::numeric_limits<double>::min();

    // Конвертируем полученные данные
    for (int i = 0; i < candlesData.size(); ++i) {
        const QList<QString>& candle = candlesData[i];
        if (candle.size() >= 5) { // timestamp, open, high, low, close
            QCPFinancialData data;

            // Преобразуем timestamp из миллисекунд в секунды (QCustomPlot использует секунды)
            double timestamp = candle[0].toDouble() / 1000.0;
            data.key = timestamp;
            timeKeys.append(timestamp);

            data.open = candle[1].toDouble();
            data.high = candle[2].toDouble();
            data.low = candle[3].toDouble();
            data.close = candle[4].toDouble();

            financialData.append(data);

            // Обновляем мин/макс для масштабирования
            minPrice = qMin(minPrice, candle[3].toDouble());
            maxPrice = qMax(maxPrice, candle[2].toDouble());
        }
    }

    // Очищаем предыдущий график
    customPlot->clearPlottables();
    customPlot->clearItems();

    // Создаем график свечей
    QCPFinancial *candlesticks = new QCPFinancial(customPlot->xAxis, customPlot->yAxis);
    candlesticks->setName("4-Hour Candles");
    candlesticks->setChartStyle(QCPFinancial::csCandlestick);
    candlesticks->data()->set(financialData);

    // Настройка свечей
    candlesticks->setWidth(4*3600*0.7); // Ширина 4-часовой свечи (70% от интервала)
    candlesticks->setTwoColored(true);
    candlesticks->setBrushPositive(QColor(0, 150, 0));
    candlesticks->setBrushNegative(QColor(150, 0, 0));
    candlesticks->setPenPositive(QPen(QColor(0, 100, 0)));
    candlesticks->setPenNegative(QPen(QColor(100, 0, 0)));

    // Настройка оси X для отображения даты/времени

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("dd.MM.yy\nhh:mm"); // Формат: дата и время с переносом
    customPlot->xAxis->setTicker(dateTicker);
    customPlot->xAxis->setLabel("Дата и время");
    customPlot->yAxis->setLabel("Цена");

    // Устанавливаем диапазоны с небольшим запасом
    double priceRange = maxPrice - minPrice;
    customPlot->yAxis->setRange(minPrice - priceRange*0.05, maxPrice + priceRange*0.05);

    if (!timeKeys.isEmpty()) {
        customPlot->xAxis->setRange(timeKeys.first() - 4*3600, timeKeys.last() + 4*3600);
    }

    // Добавляем уровни поддержки/сопротивления
    if (!qIsNaN(support)) {
        QCPItemStraightLine *supportLine = new QCPItemStraightLine(customPlot);
        supportLine->point1->setCoords(timeKeys.first(), support);
        supportLine->point2->setCoords(timeKeys.last(), support);
        supportLine->setPen(QPen(QColor(0, 0, 255), 1, Qt::DashLine));

        QCPItemText *supportText = new QCPItemText(customPlot);
        supportText->position->setCoords(timeKeys.last(), support);
        supportText->setText(QString("Support: %1").arg(support, 0, 'f', 2));
        supportText->setPositionAlignment(Qt::AlignRight|Qt::AlignVCenter);
    }

    if (!qIsNaN(resistance)) {
        QCPItemStraightLine *resistanceLine = new QCPItemStraightLine(customPlot);
        resistanceLine->point1->setCoords(timeKeys.first(), resistance);
        resistanceLine->point2->setCoords(timeKeys.last(), resistance);
        resistanceLine->setPen(QPen(QColor(255, 0, 0), 1, Qt::DashLine));

        QCPItemText *resistanceText = new QCPItemText(customPlot);
        resistanceText->position->setCoords(timeKeys.last(), resistance);
        resistanceText->setText(QString("Resistance: %1").arg(resistance, 0, 'f', 2));
        resistanceText->setPositionAlignment(Qt::AlignRight|Qt::AlignVCenter);
    }

    // Включаем легенду и обновляем график
    customPlot->legend->setVisible(true);
    customPlot->replot();

    // Выводим в лог информацию о масштабах
    qDebug() << "Price range:" << minPrice << "-" << maxPrice;
    if (!timeKeys.isEmpty()) {
        qDebug() << "Time range:" << QDateTime::fromSecsSinceEpoch(timeKeys.first()).toString()
        << "-" << QDateTime::fromSecsSinceEpoch(timeKeys.last()).toString();
    }
}
// void InfoMainWindow::setupCandlestickChart()
// {
//     // Подготовка данных (пример)
//     QVector<QCPFinancialData> data;
//     data << QCPFinancialData(1, 10, 13, 9, 11);  // time, open, high, low, close
//     data << QCPFinancialData(2, 11, 15, 10, 14);
//     data << QCPFinancialData(3, 14, 16, 12, 13);
//     data << QCPFinancialData(4, 13, 14, 11, 12);
//     data << QCPFinancialData(5, 12, 13, 9, 10);

//     // Создание графика свечей
//     QCPFinancial *candlesticks = new QCPFinancial(customPlot->xAxis, customPlot->yAxis);
//     candlesticks->setName("Candlesticks");
//     candlesticks->setChartStyle(QCPFinancial::csCandlestick);
//     candlesticks->data()->set(data);

//     // Настройка цветов
//     candlesticks->setWidth(0.5);
//     candlesticks->setTwoColored(true);
//     candlesticks->setBrushPositive(QColor(0, 255, 0)); // Зеленый для роста
//     candlesticks->setBrushNegative(QColor(255, 0, 0)); // Красный для падения
//     candlesticks->setPenPositive(QPen(QColor(0, 180, 0)));
//     candlesticks->setPenNegative(QPen(QColor(180, 0, 0)));

//     // Настройка осей
//     customPlot->xAxis->setLabel("Time");
//     customPlot->yAxis->setLabel("Price");
//     customPlot->xAxis->setRange(0, 85); // Подберите диапазон под ваши данные
//     customPlot->yAxis->setRange(8, 17); // support resistance

//     // Включаем легенду и обновляем график
//     customPlot->legend->setVisible(true);
//     customPlot->replot();
// }
