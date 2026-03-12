#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "tvbg_na.h"
#include "tvbg_eu.h"

// ===== НАСТРОЙКИ ТОЧКИ ДОСТУПА =====
const char* ap_ssid = "IR_Power_250";
const char* ap_password = "12345678";

// ===== ПИНЫ =====
const uint16_t IR_LED_PIN = 5;        // D1
const uint16_t STATUS_LED_PIN = 2;     // D4 (встроенный светодиод, LOW = вкл)

// ===== НАСТРОЙКИ СКОРОСТИ =====
const unsigned long sendIntervalClassic = 200;   // для старых кодов (мс)
const unsigned long sendIntervalRaw = 350;       // для TV-B-Gone (чуть больше)
const int repeatsPerCommand = 1;                  // повторы не нужны

// ===== СТАРЫЕ ПРОТОКОЛЬНЫЕ КОДЫ (вставьте сюда свой массив) =====
struct IRCode {
  String protocol;
  String address;
  String command;
  uint8_t bits;
  String name;
  uint64_t rawData;
};

// ⚠️ ЗАМЕНИТЕ ЭТОТ МАССИВ НА ВАШ РЕАЛЬНЫЙ МАССИВ С 250+ КОДАМИ
const uint16_t NUM_CODES = 250;
IRCode powerCodes[NUM_CODES] = {
  // Примеры (замените на свои):
  {"NEC", "", "", 32, "NEC_Power", 0x20DF10EF},
  {"NEC", "", "", 32, "NEC_Vol+", 0x20DF08F7},
  {"NEC", "", "", 32, "NEC_Vol-", 0x20DF8877},
  {"SONY12", "", "", 12, "Sony_Power1", 0xA90},
  {"SONY12", "", "", 12, "Sony_Power2", 0x750},
  {"RC5", "", "", 14, "RC5_Power1", 0x10C0},
  {"RC5", "", "", 14, "RC5_Power2", 0x10A0},
  {"RC6", "", "", 20, "RC6_Power", 0x8000F240},
  {"SAMSUNG", "", "", 32, "Samsung_Power1", 0xE0E040BF},
  {"SAMSUNG", "", "", 32, "Samsung_Power2", 0xE0E0E01F},
  {"PANASONIC", "", "", 48, "Panasonic_Power", 0x40040190F40},
  {"JVC", "", "", 16, "JVC_Power", 0xC0E8},
  {"LG", "", "", 28, "LG_Power", 0x88004B4},
  {"SHARP", "", "", 15, "Sharp_Power", 0x40D0},
  {"DENON", "", "", 15, "Denon_Power", 0x2D88},

  // ----- 2. КОДЫ TV-B-GONE: СЕВЕРНАЯ АМЕРИКА/АЗИЯ (первые 15 из 115) -----
  {"NEC", "0x07", "0x02", 32, "NA_Samsung1", 0},
  {"NEC", "0x07", "0x7F", 32, "NA_Samsung2", 0},
  {"NEC", "0x04", "0x08", 32, "NA_LG1", 0},
  {"NEC", "0x04", "0x83", 32, "NA_LG2", 0},
  {"NEC", "0x01", "0x1D", 32, "NA_Sharp1", 0},
  {"NEC", "0x02", "0x3F", 32, "NA_Toshiba1", 0},
  {"NEC", "0x03", "0x5F", 32, "NA_Hitachi1", 0},
  {"NEC", "0x05", "0x1F", 32, "NA_JVC1", 0},
  {"NEC", "0x06", "0x8F", 32, "NA_Daewoo1", 0},
  {"NEC", "0x08", "0xCF", 32, "NA_Telefunken1", 0},
  {"NEC", "0x09", "0x4F", 32, "NA_Grundig1", 0},
  {"NEC", "0x0A", "0x2F", 32, "NA_Loewe1", 0},
  {"NEC", "0x0B", "0x0F", 32, "NA_Metz1", 0},
  {"NEC", "0x0C", "0x9F", 32, "NA_Nokia1", 0},
  {"NEC", "0x0D", "0x57", 32, "NA_Akai1", 0},
  // ... здесь должно быть ещё 100 NA кодов ...

  // ----- 3. КОДЫ TV-B-GONE: ЕВРОПА/АВСТРАЛИЯ/БЛИЖНИЙ ВОСТОК (первые 15 из 115) -----
  {"NEC", "0x10", "0x02", 32, "EU_Samsung1", 0},
  {"NEC", "0x10", "0x7F", 32, "EU_Samsung2", 0},
  {"NEC", "0x11", "0x08", 32, "EU_LG1", 0},
  {"NEC", "0x11", "0x83", 32, "EU_LG2", 0},
  {"NEC", "0x12", "0x1D", 32, "EU_Sharp1", 0},
  {"NEC", "0x13", "0x3F", 32, "EU_Toshiba1", 0},
  {"NEC", "0x14", "0x5F", 32, "EU_Hitachi1", 0},
  {"NEC", "0x15", "0x1F", 32, "EU_JVC1", 0},
  {"SONY12", "0x10", "0x15", 12, "EU_Sony1", 0},
  {"SONY12", "0x10", "0x75", 12, "EU_Sony2", 0},
  {"SONY12", "0x11", "0x15", 12, "EU_Sony3", 0},
  {"RC5", "0x10", "0x0C", 14, "EU_Philips1", 0},
  {"RC5", "0x11", "0x0C", 14, "EU_Philips2", 0},
  {"RC5", "0x12", "0x0C", 14, "EU_Philips3", 0},
  {"NEC", "0x16", "0x6F", 32, "EU_Mitsubishi1", 0},
  // ... ваш массив ...
};

// ===== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ =====
enum Mode {
  MODE_STOPPED,
  MODE_SEQUENTIAL,      // старые коды по порядку
  MODE_RANDOM,          // старые коды случайно
  MODE_SEQUENTIAL_NA,   // TV-B-Gone NA по порядку
  MODE_SEQUENTIAL_EU,   // TV-B-Gone EU по порядку
  MODE_RANDOM_NA,       // TV-B-Gone NA случайно
  MODE_RANDOM_EU        // TV-B-Gone EU случайно
};

Mode currentMode = MODE_STOPPED;
uint16_t seqIndex = 0;
String currentCommandName = "";

ESP8266WebServer server(80);
IRsend irsend(IR_LED_PIN);

unsigned long lastSendTime = 0;
unsigned long currentInterval = sendIntervalClassic;

// ===== МИГАНИЕ СВЕТОДИОДОМ (неблокирующее) =====
bool blinkActive = false;
int blinkTotal = 0;
int blinksDone = 0;
bool blinkState = false;
unsigned long blinkTimer = 0;
const unsigned long blinkDuration = 200;

void startBlink(int times) {
  blinkActive = true;
  blinkTotal = times;
  blinksDone = 0;
  blinkState = false;
  digitalWrite(STATUS_LED_PIN, HIGH);
  blinkTimer = millis();
}

void updateBlink() {
  if (!blinkActive) return;
  if (blinksDone >= blinkTotal * 2) {
    blinkActive = false;
    digitalWrite(STATUS_LED_PIN, HIGH);
    return;
  }
  if (millis() - blinkTimer >= blinkDuration) {
    blinkTimer = millis();
    blinkState = !blinkState;
    digitalWrite(STATUS_LED_PIN, blinkState ? LOW : HIGH);
    blinksDone++;
  }
}

// ===== ОТПРАВКА СТАРОЙ КОМАНДЫ =====
void sendSingleCommand(const IRCode &code) {
  if (code.rawData != 0) {
    // По умолчанию шлём как NEC, но можно расширить под другие протоколы
    irsend.sendNEC(code.rawData, code.bits);
  } else {
    // Если используете address/command, преобразуйте здесь
    uint64_t addr = strtoll(code.address.c_str(), NULL, 16);
    uint64_t cmd = strtoll(code.command.c_str(), NULL, 16);
    uint32_t necData = irsend.encodeNEC(addr, cmd);
    irsend.sendNEC(necData, 32);
  }
  Serial.printf("Sent %s\n", code.name.c_str());
}

void sendCommandWithRepeats(const IRCode &code, int repeats) {
  for (int r = 0; r < repeats; r++) {
    sendSingleCommand(code);
    delay(50);
  }
  currentCommandName = code.name;
}

// ===== ОТПРАВКА RAW-ПАТТЕРНА TV-B-Gone (с копированием из PROGMEM) =====
void sendTVBGCode(const uint16_t* data_P, uint16_t len, uint16_t freq) {
  // Создаём временный буфер в RAM
  uint16_t buffer[len];
  // Копируем данные из PROGMEM в RAM
  memcpy_P(buffer, data_P, len * sizeof(uint16_t));
  // Отправляем из RAM
  irsend.sendRaw(buffer, len, freq);
  Serial.printf("Sent %s, len=%d, freq=%d\n", currentCommandName.c_str(), len, freq);
}

// ===== УСТАНОВКА РЕЖИМА =====
void setMode(Mode newMode) {
  if (currentMode == newMode) return;
  currentMode = newMode;
  seqIndex = 0;
  currentInterval = (newMode == MODE_SEQUENTIAL || newMode == MODE_RANDOM) ? sendIntervalClassic : sendIntervalRaw;
  startBlink(newMode != MODE_STOPPED ? 2 : 1);
  currentCommandName = "";
}

// ===== ВЕБ-ОБРАБОТЧИКИ =====
void handleMode() {
  if (server.hasArg("value")) {
    String val = server.arg("value");
    if (val == "seq") setMode(MODE_SEQUENTIAL);
    else if (val == "rand") setMode(MODE_RANDOM);
    else if (val == "seq_na") setMode(MODE_SEQUENTIAL_NA);
    else if (val == "seq_eu") setMode(MODE_SEQUENTIAL_EU);
    else if (val == "rand_na") setMode(MODE_RANDOM_NA);
    else if (val == "rand_eu") setMode(MODE_RANDOM_EU);
    else if (val == "stop") setMode(MODE_STOPPED);
  }
  String json = "{";
  json += "\"mode\":\"" + String(currentMode == MODE_SEQUENTIAL ? "seq" :
                               (currentMode == MODE_RANDOM ? "rand" :
                               (currentMode == MODE_SEQUENTIAL_NA ? "seq_na" :
                               (currentMode == MODE_SEQUENTIAL_EU ? "seq_eu" :
                               (currentMode == MODE_RANDOM_NA ? "rand_na" :
                               (currentMode == MODE_RANDOM_EU ? "rand_eu" : "stopped")))))) + "\",";
  json += "\"index\":" + String(seqIndex) + ",";
  json += "\"total\":" + String(
    currentMode == MODE_SEQUENTIAL || currentMode == MODE_RANDOM ? NUM_CODES :
    (currentMode == MODE_SEQUENTIAL_NA || currentMode == MODE_RANDOM_NA ? NUM_NA :
     (currentMode == MODE_SEQUENTIAL_EU || currentMode == MODE_RANDOM_EU ? NUM_EU : 0))
  ) + ",";
  json += "\"name\":\"" + currentCommandName + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IR Blaster • Matrix</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: 'Courier New', monospace; background: black; color: #0f0; min-height: 100vh; display: flex; justify-content: center; align-items: center; position: relative; overflow: hidden; }
        #matrixCanvas { position: fixed; top: 0; left: 0; width: 100%; height: 100%; z-index: 0; pointer-events: none; opacity: 0.4; }
        .container { background: rgba(0, 20, 0, 0.85); backdrop-filter: blur(2px); border: 2px solid #0f0; border-radius: 20px; box-shadow: 0 0 30px #0f0; padding: 30px; max-width: 600px; width: 90%; text-align: center; z-index: 1; position: relative; }
        h1 { font-size: 2rem; margin-bottom: 0.5rem; text-shadow: 0 0 10px #0f0; letter-spacing: 2px; }
        .subtitle { font-size: 0.9rem; margin-bottom: 1.5rem; border-bottom: 1px solid #0f0; padding-bottom: 0.8rem; opacity: 0.9; }
        .section { margin: 20px 0; padding: 10px; border: 1px solid #0f0; border-radius: 10px; }
        .section h2 { font-size: 1.2rem; margin-bottom: 10px; }
        .button-group { display: flex; flex-wrap: wrap; gap: 10px; justify-content: center; }
        button { flex: 1 1 140px; padding: 12px 15px; font-family: 'Courier New', monospace; font-size: 1rem; font-weight: bold; border: none; border-radius: 50px; cursor: pointer; transition: 0.3s; text-transform: uppercase; letter-spacing: 1px; background: black; color: #0f0; border: 2px solid #0f0; box-shadow: 0 0 10px #0f0; }
        button.stop { color: #f00; border-color: #f00; box-shadow: 0 0 10px #f00; }
        button:hover { transform: scale(1.05); box-shadow: 0 0 20px currentColor; }
        .status-card { background: rgba(0, 10, 0, 0.7); border: 1px solid #0f0; border-radius: 15px; padding: 20px; margin-top: 20px; }
        #status { font-size: 1.3rem; min-height: 2rem; }
        #commandName { font-size: 1rem; color: #0f0; margin-top: 10px; }
        .progress-bar { width: 100%; height: 20px; background: #111; border: 1px solid #0f0; border-radius: 10px; margin: 20px 0 10px; overflow: hidden; }
        #progressFill { height: 100%; width: 0%; background: #0f0; box-shadow: 0 0 20px #0f0; transition: width 0.2s; }
        .footer { margin-top: 20px; font-size: 0.8rem; opacity: 0.6; }
    </style>
</head>
<body>
    <canvas id="matrixCanvas"></canvas>
    <div class="container">
        <h1>📡 IR BLASTER</h1>
        <div class="subtitle">Classic 250+ codes + TV-B-Gone (NA:135 / EU:122)</div>

        <div class="section">
            <h2>📀 Classic Codes</h2>
            <div class="button-group">
                <button onclick="sendMode('seq')">Sequential</button>
                <button onclick="sendMode('rand')">Random</button>
            </div>
        </div>

        <div class="section">
            <h2>🌎 TV-B-Gone NA</h2>
            <div class="button-group">
                <button onclick="sendMode('seq_na')">Sequential</button>
                <button onclick="sendMode('rand_na')">Random</button>
            </div>
        </div>

        <div class="section">
            <h2>🌍 TV-B-Gone EU</h2>
            <div class="button-group">
                <button onclick="sendMode('seq_eu')">Sequential</button>
                <button onclick="sendMode('rand_eu')">Random</button>
            </div>
        </div>

        <button class="stop" onclick="sendMode('stop')" style="width:100%; margin-top:20px;">⏹ STOP</button>

        <div class="status-card">
            <div id="status">Stopped</div>
            <div id="commandName"></div>
            <div class="progress-bar"><div id="progressFill"></div></div>
        </div>
        <div class="footer">ESP8266 • Matrix Rain • RAM buffer for TV-B-Gone</div>
    </div>

    <script>
        const canvas = document.getElementById('matrixCanvas');
        const ctx = canvas.getContext('2d');
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
        const chars = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ';
        const fontSize = 16;
        const columns = canvas.width / fontSize;
        const drops = Array(Math.floor(columns)).fill(1);
        function drawMatrix() {
            ctx.fillStyle = 'rgba(0, 0, 0, 0.05)';
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            ctx.fillStyle = '#0f0';
            ctx.font = fontSize + 'px monospace';
            for (let i = 0; i < drops.length; i++) {
                const text = chars[Math.floor(Math.random() * chars.length)];
                ctx.fillText(text, i * fontSize, drops[i] * fontSize);
                if (drops[i] * fontSize > canvas.height && Math.random() > 0.975) drops[i] = 0;
                drops[i]++;
            }
        }
        setInterval(drawMatrix, 50);

        function sendMode(mode) {
            fetch('/mode?value=' + mode).then(r => r.json()).then(data => updateUI(data));
        }
        function updateUI(data) {
            let status = 'Stopped';
            let total = data.total || 0;
            if (data.mode === 'seq') status = `Sequential (${data.index}/${total})`;
            else if (data.mode === 'rand') status = 'Random (classic)';
            else if (data.mode === 'seq_na') status = `NA Seq (${data.index}/${total})`;
            else if (data.mode === 'seq_eu') status = `EU Seq (${data.index}/${total})`;
            else if (data.mode === 'rand_na') status = 'Random NA';
            else if (data.mode === 'rand_eu') status = 'Random EU';
            document.getElementById('status').innerText = status;
            document.getElementById('commandName').innerText = data.name || '';
            let percent = total > 0 ? (data.index / total * 100) : 0;
            document.getElementById('progressFill').style.width = percent + '%';
        }
        setInterval(() => {
            fetch('/mode?value=status').then(r => r.json()).then(data => updateUI(data));
        }, 500);
    </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, HIGH);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  
  server.on("/", handleRoot);
  server.on("/mode", handleMode);
  server.begin();
  
  irsend.begin();
  startBlink(1);
}

// ===== LOOP =====
void loop() {
  server.handleClient();
  updateBlink();

  if (currentMode != MODE_STOPPED && millis() - lastSendTime >= currentInterval) {
    lastSendTime = millis();

    // Режимы со старыми кодами
    if (currentMode == MODE_SEQUENTIAL && seqIndex < NUM_CODES) {
      sendCommandWithRepeats(powerCodes[seqIndex], repeatsPerCommand);
      seqIndex++;
    } else if (currentMode == MODE_RANDOM) {
      uint16_t r = random(0, NUM_CODES);
      sendCommandWithRepeats(powerCodes[r], repeatsPerCommand);
    }

    // TV-B-Gone NA
    else if (currentMode == MODE_SEQUENTIAL_NA && seqIndex < NUM_NA) {
      const uint16_t* data = (const uint16_t*)pgm_read_ptr(&na_patterns[seqIndex]);
      uint16_t len = pgm_read_word(&na_lengths[seqIndex]);
      uint16_t freq = pgm_read_word(&na_freqs[seqIndex]);
      currentCommandName = "NA-" + String(seqIndex);
      sendTVBGCode(data, len, freq);
      seqIndex++;
    } else if (currentMode == MODE_RANDOM_NA) {
      uint16_t r = random(0, NUM_NA);
      const uint16_t* data = (const uint16_t*)pgm_read_ptr(&na_patterns[r]);
      uint16_t len = pgm_read_word(&na_lengths[r]);
      uint16_t freq = pgm_read_word(&na_freqs[r]);
      currentCommandName = "NA-random";
      sendTVBGCode(data, len, freq);
    }

    // TV-B-Gone EU
    else if (currentMode == MODE_SEQUENTIAL_EU && seqIndex < NUM_EU) {
      const uint16_t* data = (const uint16_t*)pgm_read_ptr(&eu_patterns[seqIndex]);
      uint16_t len = pgm_read_word(&eu_lengths[seqIndex]);
      uint16_t freq = pgm_read_word(&eu_freqs[seqIndex]);
      currentCommandName = "EU-" + String(seqIndex);
      sendTVBGCode(data, len, freq);
      seqIndex++;
    } else if (currentMode == MODE_RANDOM_EU) {
      uint16_t r = random(0, NUM_EU);
      const uint16_t* data = (const uint16_t*)pgm_read_ptr(&eu_patterns[r]);
      uint16_t len = pgm_read_word(&eu_lengths[r]);
      uint16_t freq = pgm_read_word(&eu_freqs[r]);
      currentCommandName = "EU-random";
      sendTVBGCode(data, len, freq);
    }

    // Автоматическое переключение в случайный режим после окончания последовательного
    if (currentMode == MODE_SEQUENTIAL && seqIndex >= NUM_CODES) setMode(MODE_RANDOM);
    if (currentMode == MODE_SEQUENTIAL_NA && seqIndex >= NUM_NA) setMode(MODE_RANDOM_NA);
    if (currentMode == MODE_SEQUENTIAL_EU && seqIndex >= NUM_EU) setMode(MODE_RANDOM_EU);
  }
}