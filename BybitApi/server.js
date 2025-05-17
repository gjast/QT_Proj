import express from "express";
import cors from "cors";
import bodyParser from "body-parser";
import fs from "fs/promises";
import path from "path";
import * as TradingModules from "./main.js";

const app = express();
app.use(cors());
app.use(bodyParser.json());

// Глобальное хранилище для клиентов
let bybitClient = null;
let trade = null;
let bot = null;
let currentKeys = {
  API_KEY: null,
  API_SECRET: null
};

// Функция инициализации/переинициализации сервисов
const initializeServices = (apiKey, apiSecret) => {
  try {
    bybitClient = new TradingModules.BybitClient(apiKey, apiSecret);
    trade = new TradingModules.Trade(bybitClient);
    bot = new TradingModules.TradingBot(bybitClient, trade);
    currentKeys = { API_KEY: apiKey, API_SECRET: apiSecret };
    console.log("Сервисы успешно инициализированы");
    return true;
  } catch (error) {
    console.error("Ошибка инициализации сервисов:", error);
    return false;
  }
};

// Загрузка ключей из файла при старте
const loadKeysFromFile = async () => {
  try {
    const envPath = path.resolve(process.cwd(), '.env');
    const data = await fs.readFile(envPath, 'utf8');
    const apiKeyMatch = data.match(/API_KEY="([^"]+)"/);
    const apiSecretMatch = data.match(/API_SECRET="([^"]+)"/);
    
    if (apiKeyMatch && apiSecretMatch) {
      return {
        apiKey: apiKeyMatch[1],
        apiSecret: apiSecretMatch[1]
      };
    }
    return null;
  } catch (error) {
    console.log("Файл .env не найден или поврежден, будет создан новый");
    return null;
  }
};

// Инициализация при запуске
(async () => {
  const keys = await loadKeysFromFile();
  if (keys) {
    initializeServices(keys.apiKey, keys.apiSecret);
  }
})();

app.post("/api/set-keys", async (req, res) => {
  const { apiKey, apiSecret } = req.body;

  if (!apiKey || !apiSecret) {
    return res.status(400).json({
      success: false,
      error: "Необходимо предоставить apiKey и apiSecret"
    });
  }

  try {
    // Сохраняем в файл
    await fs.writeFile(
      path.resolve(process.cwd(), '.env'),
      `API_KEY="${apiKey}"\nAPI_SECRET="${apiSecret}"`
    );

    // Инициализируем сервисы с новыми ключами
    const success = initializeServices(apiKey, apiSecret);
    
    if (success) {
      return res.json({ success: true, message: "Ключи успешно обновлены" });
    } else {
      return res.status(500).json({
        success: false,
        error: "Ошибка при инициализации сервисов с новыми ключами"
      });
    }
  } catch (error) {
    console.error("Ошибка сохранения ключей:", error);
    return res.status(500).json({
      success: false,
      error: "Ошибка при сохранении ключей"
    });
  }
});

// Роут для проверки текущих ключей
app.get("/api/current-keys", (req, res) => {
  res.json({
    hasKeys: !!currentKeys.API_KEY,
    keyLastChars: currentKeys.API_KEY ? `...${currentKeys.API_KEY.slice(-4)}` : null
  });
});

// Роут для запуска бота
app.post("/start-bot", async (req, res) => {
  if (!bot) {
    return res.status(400).json({
      success: false,
      error: "API ключи не установлены"
    });
  }

  const { symbol } = req.body;

  if (!symbol) {
    return res.status(400).json({
      success: false,
      error: "Необходимо указать symbol"
    });
  }

  try {
    const result = await bot.Trade(symbol, "linear");
    res.json({ shouldBuy: result, message: "Бот запущен" });
  } catch (error) {
    console.error("Ошибка при запуске бота:", error);
    res.status(500).json({ success: false, error: error.message });
  }
});

// Роут для получения рыночных данных
app.get("/market-data/:symbol", async (req, res) => {
  if (!trade) {
    return res.status(400).json({ error: "API ключи не установлены" });
  }

  const { symbol } = req.params;
  const orderType = req.query.orderType || "spot";

  try {
    const data = await trade.getClines(orderType, symbol);
    res.json({
      candles: data.candles,
      rsi: data.rsi,
      macd: data.macd,
      stoch: data.stoch,
      support: data.levels.support,
      resistance: data.levels.resistance,
    });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Роут для получения баланса
app.get("/balance", async (req, res) => {
  if (!bybitClient) {
    return res.status(400).json({ error: "API ключи не установлены" });
  }

  try {
    const balance = await bybitClient.getBalances();
    res.json(balance);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Роут для получения цены
app.get("/price/:symbol", async (req, res) => {
  if (!bybitClient) {
    return res.status(400).json({ error: "API ключи не установлены" });
  }

  const { symbol } = req.params;
  const orderType = req.query.orderType || "spot";

  try {
    const price = await bybitClient.getPrice(orderType, symbol);
    res.json({ price });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Роут для отмены всех ордеров
app.post("/cancel-orders", async (req, res) => {
  if (!bybitClient) {
    return res.status(400).json({ error: "API ключи не установлены" });
  }

  try {
    await bybitClient.cancelAllOrders();
    res.json({ success: true, message: "Все ордера отменены" });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Роут для размещения ордера с плечом
app.post("/place-leveraged-order", async (req, res) => {
  if (!bybitClient) {
    return res.status(400).json({ error: "API ключи не установлены" });
  }

  const { symbol, side, quantity, takeProfit, stopLoss } = req.body;

  try {
    await bybitClient.placeLeveragedOrder(
      symbol,
      side,
      quantity,
      takeProfit,
      stopLoss
    );
    res.json({ success: true, message: `Ордер ${side} размещен` });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Запуск сервера
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Сервер запущен на порту ${PORT}`);
});