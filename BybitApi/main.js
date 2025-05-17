import { RestClientV5 } from "bybit-api";
import ti from "technicalindicators";

class BybitClient {
  constructor(apiKey, apiSecret) {
    this.client = new RestClientV5({
      key: apiKey,
      secret: apiSecret,
      enable_time_sync: true,
      testnet: false,
    });
  }

  async getBalances() {
    try {
      const response = await this.client.getWalletBalance({
        accountType: "UNIFIED",
      });
      return {
        listBalance: response.result,
        totalBalance: parseFloat(
          response.result.list[0].totalWalletBalance
        ).toFixed(2),

        USDT: response.result.list[0].coin.filter((el) => {
          return el.coin === "USDT";
        })[0].walletBalance,
      };
    } catch (error) {
      console.error("Ошибка при получении баланса:", error);
      throw error;
    }
  }

  async getPrice(category, symbol) {
    try {
      const response = await this.client.getTickers({
        category: category,
        symbol: symbol,
      });
      return response.result.list[0].lastPrice;
    } catch (error) {
      console.error("Ошибка при получении цены:", error);
      throw error;
    }
  }

  async setLeverage(symbol, leverage) {
    try {
      await this.client.setLeverage({
        category: "linear",
        symbol,
        buyLeverage: leverage,
        sellLeverage: leverage,
      });
      console.log(`Плечо успешно установлено на ${leverage}x`);
    } catch (error) {
      console.error("Ошибка при установке плеча:", error);
      throw error;
    }
  }

  async placeLeveragedOrder(symbol, side, qty, takeProfit, stopLoss) {
    try {
      await this.setLeverage(symbol, 10);
      

      const orderResponse = await this.client.submitOrder({
        category: "linear",
        symbol: symbol,
        side: side,
        orderType: "Market",
        qty: ((qty / (await this.getPrice("linear", symbol)))
          .toFixed(0))
          .toString(),
        positionIdx: 0,
        leverage: '10',
        takeProfit: takeProfit,
        stopLoss: stopLoss,
      });
      console.log(
        `Фьючерсный ордер с плечом ${10}x успешно размещен:`,
        orderResponse
      );
      return orderResponse;
    } catch (error) {
      console.error("Ошибка при размещении ордера:", error);
      throw error;
    } finally {
      console.log(`Завершено размещение ордера с плечом ${10}x`);
    }
  }

  async placeSpotOrder(symbol, side, qty) {
    try {
      const orderResponse = await this.client.submitOrder({
        category: "spot",
        symbol: symbol,
        side: side,
        orderType: "Market",
        qty: qty,
        marketUnit: "quoteCoin",
      });
      console.log("Спотовый ордер успешно размещен:", orderResponse);
      return orderResponse;
    } catch (error) {
      console.error("Ошибка при размещении спотового ордера:", error);
      throw error;
    }
  }

  async cancelAllOrders() {
    try {
      const res = await this.client.cancelAllOrders({
        category: "linear",
        settleCoin: "USDT",
      });
      console.log("Все фьючерсные ордера отменены:", res);
      const resSpot = await this.client.cancelAllOrders({
        category: "spot",
      });
      console.log("Все спотовые ордера отменены:", resSpot);

      const coinsWithNonZeroValue = (
        await this.getBalances()
      ).listBalance.list[0].coin
        .filter(
          ({ usdValue, coin }) => parseFloat(usdValue) > 0 && coin !== "USDT"
        )
        .map(({ coin, usdValue }) => ({ coin, usdValue }));

      coinsWithNonZeroValue.length > 0
        ? coinsWithNonZeroValue.forEach((el) => {
            this.placeSpotOrder(
              `${el.coin}USDT`,
              "Sell",
              parseInt(el.usdValue).toFixed(5).toString()
            );
          })
        : console.log("no coin");
    } catch (error) {
      console.error("Ошибка при отмене ордеров:", error);
      throw error;
    }
  }
}

class Trade {
  constructor(bybitClient) {
    this.client = bybitClient;
  }

  async getClines(category, symbol) {
    try {
      const response = await this.client.client.getKline({
        category: category,
        symbol: symbol,
        interval: "240",
        limit: 84,
      });

      const candles = response.result.list.reverse();

      const closes = candles.map((candle) => parseFloat(candle[4]));
      const highs = candles.map((candle) => parseFloat(candle[2]));
      const lows = candles.map((candle) => parseFloat(candle[3]));

      const rsi = ti.rsi({ values: closes, period: 14 });
      const macd = ti.MACD.calculate({
        values: closes,
        fastPeriod: 12,
        slowPeriod: 26,
        signalPeriod: 9,
        SimpleMAOscillator: false,
        SimpleMASignal: false,
      });

      const last20Candles = candles.slice(-20);
      const stochCloses = last20Candles.map((candle) => parseFloat(candle[4]));
      const stochHighs = last20Candles.map((candle) => parseFloat(candle[2]));
      const stochLows = last20Candles.map((candle) => parseFloat(candle[3]));

      const stoch = ti.Stochastic.calculate({
        high: stochHighs,
        low: stochLows,
        close: stochCloses,
        period: 14,
        signalPeriod: 3,
      });

      const levels = this.findSupportResistance(response.result.list);

      return {
        candles: response.result.list,
        levels: levels,
        rsi: rsi[rsi.length - 1],
        macd: macd[macd.length - 1],
        stoch: stoch[stoch.length - 1],
      };
    } catch (error) {
      console.error("Ошибка при получении свечей:", error);
      throw error;
    }
  }

  findSupportResistance(candles) {
    if (!candles || candles.length === 0) {
      return { support: null, resistance: null };
    }

    let minLow = parseFloat(candles[0][3]);
    let maxHigh = parseFloat(candles[0][2]);

    for (const candle of candles) {
      const low = parseFloat(candle[3]);
      const high = parseFloat(candle[2]);

      if (low < minLow) minLow = low;
      if (high > maxHigh) maxHigh = high;
    }

    return {
      support: parseFloat(minLow.toFixed(2)),
      resistance: parseFloat(maxHigh.toFixed(2)),
    };
  }

 shouldBuyBeforeImpulse(data) {
  const { rsi, macd, stoch, levels, candles } = data;

  // 1. Проверка RSI (перепроданность)
  const isOversold = rsi < 30;

  // 2. Проверка MACD (бычий сигнал)
  const isMacdBullish = macd.histogram > 0 && macd.macd > macd.signal;

  // 3. Проверка Stochastic (перепроданность + бычье пересечение)
  const isStochBullish = stoch.k < 20 && stoch.d < 20 && stoch.k > stoch.d;

  // 4. Проверка объема (спайк)
  const lastVolume = parseFloat(candles.at(-1)?.[5] || 0);
  const avgVolume = candles.reduce((sum, candle) => sum + parseFloat(candle[5]), 0) / candles.length;
  const isVolumeSpike = lastVolume > avgVolume * 1.5;

  // 5. Проверка уровня поддержки (с защитой от undefined)
  const lastClose = parseFloat(candles.at(-1)?.[4] || 0);
  const supportLevel = levels?.support;
  const nearSupport = supportLevel 
    ? lastClose <= supportLevel * 1.01 && lastClose >= supportLevel * 0.99 
    : false;

  // 6. Проверка тренда через MA(50)
  const closes = candles.map(candle => parseFloat(candle[4]));
  const ma50 = closes.length >= 50 
    ? closes.slice(-50).reduce((sum, price) => sum + price, 0) / 50 
    : null;
  const isUptrend = ma50 !== null ? lastClose >= ma50 : false;

  // 7. Дополнительные условия (например, если MACD сильно bullish)
  const isMacdStrongBullish = macd.histogram > 0.5 && macd.macd - macd.signal > 0.2;

  return (
    // Сценарий 1: Сильная перепроданность + отскок от поддержки
    (isOversold && isStochBullish) ||
    // Сценарий 2: MACD сильный бычий + тренд вверх
    (isMacdStrongBullish && isUptrend) ||
    // Сценарий 3: Перепроданность + всплеск объема
    (isOversold && isVolumeSpike) ||
    // Сценарий 4: Stochastic + MACD бычий (без привязки к RSI)
    (isStochBullish && isMacdBullish) ||
    // Сценарий 5: Просто сильный всплеск объема у поддержки
    (isVolumeSpike && nearSupport)
  );
}
}

class TradingBot {
  constructor(bybitClient, trade) {
    this.client = bybitClient; // other class for work api
    this.trade = trade; //other class for work strategy
  }

  async Trade(symbol, order) {
  return this.trade.getClines(order, symbol).then((data) => {
    return this.trade.shouldBuyBeforeImpulse(data); 
  });
}

  async startTrade() {
     await this.Trade().then((data) =>{
      return data
     });
  }
}



export { BybitClient, Trade, TradingBot };

