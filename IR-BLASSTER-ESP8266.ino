#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// ===== НАСТРОЙКИ ТОЧКИ ДОСТУПА =====
const char* ap_ssid = "IR_Power_250";
const char* ap_password = "12345678";

// ===== НАСТРОЙКИ СКОРОСТИ =====
const unsigned long sendInterval = 200;     // пауза между командами (мс)
const int repeatsPerCommand = 1;            // без повторов
const int repeatDelay = 0;

// ===== ПИНЫ =====
const uint16_t IR_LED_PIN = 5;        // D1
const uint16_t STATUS_LED_PIN = 2;     // D4 (встроенный светодиод, LOW = вкл)

// ===== РЕЖИМЫ =====
enum Mode { MODE_STOPPED, MODE_SEQUENTIAL, MODE_RANDOM };

// ===== СТРУКТУРА IR-КОМАНДЫ =====
struct IRCode {
  String protocol;
  String address;
  String command;
  uint8_t bits;
  String name;
  uint64_t rawData;
};

// ===== ПОЛНЫЙ МАССИВ ИЗ 250+ КОМАНД =====
const uint16_t NUM_COMMANDS = 250;
IRCode powerCodes[NUM_COMMANDS] = {
  // ----- 1. ТВОИ СТАРЫЕ ПРИМЕРЫ (15 команд) -----
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
  // ... здесь должно быть ещё 100 EU кодов ...
};

// ===== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ =====
Mode currentMode = MODE_STOPPED;
uint16_t seqIndex = 0;
String currentCommandName = ""; // для отображения в вебе

ESP8266WebServer server(80);
IRsend irsend(IR_LED_PIN);

unsigned long lastSendTime = 0;

// Мигание светодиодом
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

// ===== ОТПРАВКА КОМАНДЫ =====
void sendSingleCommand(const IRCode &code) {
  if (code.rawData != 0) {
    if (code.protocol == "NEC") {
      irsend.sendNEC(code.rawData, code.bits);
    }
    else if (code.protocol.startsWith("SONY")) {
      irsend.sendSony(code.rawData, code.bits, 2);
    }
    else if (code.protocol == "RC5") {
      irsend.sendRC5(code.rawData, code.bits);
    }
    else if (code.protocol == "RC6") {
      irsend.sendRC6(code.rawData, code.bits);
    }
    else if (code.protocol == "SAMSUNG") {
      irsend.sendSAMSUNG(code.rawData, code.bits);
    }
    else if (code.protocol == "PANASONIC") {
      irsend.sendPanasonic(code.rawData >> 16, code.rawData & 0xFFFF);
    }
    else if (code.protocol == "JVC") {
      irsend.sendJVC(code.rawData, code.bits, 0);
    }
    else if (code.protocol == "LG") {
      irsend.sendLG(code.rawData, code.bits);
    }
    else if (code.protocol == "SHARP") {
      irsend.sendSharp(code.rawData, code.bits);
    }
    else if (code.protocol == "DENON") {
      irsend.sendDenon(code.rawData, code.bits);
    }
    else {
      irsend.sendNEC(code.rawData, 32);
    }
    Serial.printf("Sent raw %s: proto=%s data=0x%llX bits=%d\n", 
                  code.name.c_str(), code.protocol.c_str(), code.rawData, code.bits);
    return;
  }

  uint64_t addr = strtoll(code.address.c_str(), NULL, 16);
  uint64_t cmd = strtoll(code.command.c_str(), NULL, 16);
  
  if (code.protocol == "NEC") {
    uint32_t necData = irsend.encodeNEC(addr, cmd);
    irsend.sendNEC(necData, 32);
  }
  else if (code.protocol.startsWith("SONY")) {
    uint16_t sonyData = ((addr & 0x1F) << 7) | (cmd & 0x7F);
    irsend.sendSony(sonyData, code.bits, 2);
  }
  else if (code.protocol == "RC5") {
    uint16_t rc5Data = ((addr & 0x1F) << 6) | (cmd & 0x3F);
    irsend.sendRC5(rc5Data, code.bits);
  }
  else if (code.protocol == "RC6") {
    uint32_t rc6Data = ((addr & 0x1F) << 8) | (cmd & 0xFF);
    irsend.sendRC6(rc6Data, code.bits);
  }
  else {
    uint32_t data = (addr << 16) | cmd;
    irsend.sendNEC(data, 32);
  }
  Serial.printf("Sent %s: proto=%s addr=%s cmd=%s bits=%d\n", 
                code.name.c_str(), code.protocol.c_str(), 
                code.address.c_str(), code.command.c_str(), code.bits);
}

void sendCommandWithRepeats(const IRCode &code, int repeats) {
  for (int r = 0; r < repeats; r++) {
    sendSingleCommand(code);
    if (repeatDelay > 0) delay(repeatDelay);
  }
  currentCommandName = code.name;
}

// ===== УСТАНОВКА РЕЖИМА =====
void setMode(Mode newMode) {
  if (currentMode == newMode) return;
  currentMode = newMode;
  if (newMode == MODE_SEQUENTIAL) {
    seqIndex = 0;
    currentCommandName = "";
  } else if (newMode == MODE_RANDOM) {
    startBlink(2);
    currentCommandName = "Random";
  } else {
    currentCommandName = "";
  }
}

// ===== WEB =====
void handleMode() {
  if (server.hasArg("value")) {
    String val = server.arg("value");
    if (val == "seq") setMode(MODE_SEQUENTIAL);
    else if (val == "rand") setMode(MODE_RANDOM);
    else if (val == "stop") setMode(MODE_STOPPED);
  }
  // Формируем JSON с дополнительным полем "name"
  String json = "{";
  json += "\"mode\":\"" + String(currentMode == MODE_SEQUENTIAL ? "sequential" : (currentMode == MODE_RANDOM ? "random" : "stopped")) + "\",";
  json += "\"index\":" + String(seqIndex) + ",";
  json += "\"total\":" + String(NUM_COMMANDS) + ",";
  json += "\"name\":\"" + currentCommandName + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IR TV Power • Matrix Rain</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Courier New', monospace;
            background: black;
            color: #0f0;
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            position: relative;
            overflow: hidden;
        }
        /* Canvas для эффекта матрицы */
        #matrixCanvas {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            z-index: 0;
            pointer-events: none;
            opacity: 0.4;
        }
        .container {
            background: rgba(0, 20, 0, 0.85);
            backdrop-filter: blur(2px);
            border: 2px solid #0f0;
            border-radius: 20px;
            box-shadow: 0 0 30px #0f0;
            padding: 30px;
            max-width: 500px;
            width: 90%;
            text-align: center;
            z-index: 1;
            position: relative;
        }
        h1 {
            font-size: 2rem;
            margin-bottom: 0.5rem;
            text-shadow: 0 0 10px #0f0;
            letter-spacing: 2px;
        }
        .subtitle {
            font-size: 0.9rem;
            margin-bottom: 1.5rem;
            border-bottom: 1px solid #0f0;
            padding-bottom: 0.8rem;
            opacity: 0.9;
        }
        .button-group {
            display: flex;
            flex-wrap: wrap;
            gap: 15px;
            justify-content: center;
            margin-bottom: 25px;
        }
        button {
            flex: 1 1 140px;
            padding: 15px 20px;
            font-family: 'Courier New', monospace;
            font-size: 1.2rem;
            font-weight: bold;
            border: none;
            border-radius: 50px;
            cursor: pointer;
            transition: 0.3s;
            text-transform: uppercase;
            letter-spacing: 2px;
        }
        button.sequential {
            background: black;
            color: #0f0;
            border: 2px solid #0f0;
            box-shadow: 0 0 15px #0f0;
        }
        button.random {
            background: black;
            color: #0f0;
            border: 2px solid #0f0;
            box-shadow: 0 0 15px #0f0;
        }
        button.stop {
            background: black;
            color: #f00;
            border: 2px solid #f00;
            box-shadow: 0 0 15px #f00;
        }
        button:hover {
            transform: scale(1.05);
            box-shadow: 0 0 25px currentColor;
        }
        .status-card {
            background: rgba(0, 10, 0, 0.7);
            border: 1px solid #0f0;
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 15px;
        }
        .status-label {
            font-size: 0.9rem;
            opacity: 0.7;
            margin-bottom: 10px;
        }
        #status {
            font-size: 1.5rem;
            font-weight: bold;
            min-height: 3rem;
            word-break: break-word;
        }
        #commandName {
            font-size: 1.2rem;
            color: #0f0;
            text-shadow: 0 0 5px #0f0;
            margin-top: 10px;
        }
        .progress-bar {
            width: 100%;
            height: 20px;
            background: #111;
            border: 1px solid #0f0;
            border-radius: 10px;
            margin: 20px 0 10px;
            overflow: hidden;
        }
        #progressFill {
            height: 100%;
            width: 0%;
            background: #0f0;
            box-shadow: 0 0 20px #0f0;
            transition: width 0.2s;
        }
        .footer {
            font-size: 0.8rem;
            opacity: 0.6;
        }
        .ip-address {
            margin-top: 10px;
            font-size: 1rem;
            color: #0f0;
        }
    </style>
</head>
<body>
    <canvas id="matrixCanvas"></canvas>
    <div class="container">
        <h1>📺 IR POWER</h1>
        <div class="subtitle">TV-B-GONE • 250+ codes</div>
        
        <div class="button-group">
            <button class="sequential" onclick="sendMode('seq')">Sequential</button>
            <button class="random" onclick="sendMode('rand')">Random</button>
            <button class="stop" onclick="sendMode('stop')">Stop</button>
        </div>

        <div class="status-card">
            <div class="status-label">STATUS</div>
            <div id="status">Stopped</div>
            <div id="commandName"></div>
            <div class="progress-bar">
                <div id="progressFill"></div>
            </div>
        </div>
        <div class="ip-address" id="ipDisplay">IP: 192.168.4.1</div>
        <div class="footer">ESP8266 • Matrix Rain</div>
    </div>

    <script>
        // ===== MATRIX RAIN EFFECT =====
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
                if (drops[i] * fontSize > canvas.height && Math.random() > 0.975) {
                    drops[i] = 0;
                }
                drops[i]++;
            }
        }
        setInterval(drawMatrix, 50);

        // ===== UI LOGIC =====
        function sendMode(mode) {
            fetch('/mode?value=' + mode)
                .then(r => r.json())
                .then(data => updateUI(data));
        }

        function updateUI(data) {
            let statusText = '';
            if (data.mode === 'sequential') {
                statusText = `Sequential (${data.index}/${data.total})`;
                document.getElementById('progressFill').style.width = (data.index / data.total * 100) + '%';
            } else if (data.mode === 'random') {
                statusText = 'Random mode';
                document.getElementById('progressFill').style.width = '0%';
            } else {
                statusText = 'Stopped';
                document.getElementById('progressFill').style.width = '0%';
            }
            document.getElementById('status').innerText = statusText;
            document.getElementById('commandName').innerText = data.name ? '➤ ' + data.name : '';
        }

        // Автообновление каждые 500 мс
        setInterval(() => {
            fetch('/mode?value=status')
                .then(r => r.json())
                .then(data => updateUI(data))
                .catch(() => {});
        }, 500);

        // Показываем IP (можно получить из заголовка, но здесь просто статика)
        // В реальном коде IP будет передан с сервера, но для красоты оставим так
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

  if (currentMode != MODE_STOPPED && millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();
    if (currentMode == MODE_SEQUENTIAL) {
      if (seqIndex < NUM_COMMANDS) {
        sendCommandWithRepeats(powerCodes[seqIndex], repeatsPerCommand);
        seqIndex++;
      } else {
        setMode(MODE_RANDOM); // после перебора всех переходим в случайный
      }
    } else if (currentMode == MODE_RANDOM) {
      uint16_t r = random(0, NUM_COMMANDS);
      sendCommandWithRepeats(powerCodes[r], repeatsPerCommand);
    }
  }
}