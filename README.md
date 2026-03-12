# IR BLASTER ESP8266

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**ESP8266-based infrared transmitter** with support for **250+ classic codes** and **full TV-B-Gone database (NA + EU)**.  
Web interface with Matrix "digital rain" animation, real‑time progress bar, and instant stop.

---

## 🇬🇧 English

### ✨ Features
- 📡 **250+ classic power codes** (NEC, Sony, RC5, etc.) – your existing database.
- 🌍 **Full TV-B-Gone NA (135 codes) & EU (122 codes)** – original raw patterns extracted from the official Java source.
- 🌐 **Beautiful web interface** with Matrix digital rain, progress bar, current command display.
- 🎮 **Modes**: Sequential / Random for each code set (classic, NA, EU).
- 🛑 **Instant Stop** – responsive even during transmission.
- 💡 Built‑in LED blinks to indicate status (double blink on mode change).
- 📶 Standalone **Wi‑Fi access point** – no router needed.

### 🔧 Hardware Required
- ESP8266 board (NodeMCU, Wemos D1 Mini, etc.)
- IR transmitter module (e.g. KY‑005) **or** discrete components (IR LED + transistor + resistors)

### 📥 Installation & Usage

#### 1. Get the TV-B-Gone codes (if you want them)
Run the provided Python script `extract_tvbg.py` on the original Java file (`TV-B-Gone_IR_Codes.java`) to generate `tvbg_na.h` and `tvbg_eu.h`.  
These files contain all raw patterns stored in **PROGMEM**.

#### 2. Prepare the sketch
- Place `tvbg_na.h` and `tvbg_eu.h` in the sketch folder.
- Open `IR_BLASTER.ino` in Arduino IDE.
- **Replace the example `powerCodes` array** with your own 250+ codes (or keep the example for testing).
- Adjust pin numbers if needed (`IR_LED_PIN` = 5 → D1; `STATUS_LED_PIN` = 2 → built‑in LED).

#### 3. Install required libraries
- **ESP8266 board support** (via Board Manager)
- **IRremoteESP8266** (via Library Manager)

#### 4. Upload and use
- Select your board (e.g. NodeMCU 1.0) and port, then upload.
- After boot, connect to Wi‑Fi network **`IR_Power_250`** (password `12345678`).
- Open browser at **`http://192.168.4.1`**.
- Choose a mode:
  - **Classic Sequential / Random** – your 250+ codes.
  - **NA Sequential / Random** – 135 North American TV‑B‑Gone patterns.
  - **EU Sequential / Random** – 122 European patterns.
- Press **STOP** to halt transmission immediately.

### 📁 Code Structure
- `IR_BLASTER.ino` – main sketch with web server, mode handling, IR send functions.
- `tvbg_na.h`, `tvbg_eu.h` – generated headers containing all raw patterns in PROGMEM.
- `extract_tvbg.py` – Python script to extract and convert patterns from the original Java source.

### ⚙️ Customisation
- **Send speed**: change `sendIntervalClassic` and `sendIntervalRaw` (in milliseconds).
- **Add your own codes**: append to the `powerCodes` array (use the same `IRCode` structure).
- **Adjust Wi‑Fi credentials**: modify `ap_ssid` and `ap_password` at the top of the sketch.

### 📜 License
MIT License – see [LICENSE](LICENSE) file.

### 🙏 Acknowledgments
- [TV-B-Gone](https://github.com/sambowenh/TV-B-Gone-UK-Edition) for the original IR codes.
- [IRremoteESP8266 library](https://github.com/crankyoldgit/IRremoteESP8266) by crankyoldgit.
- Bruce firmware for inspiration.

---

## 🇷🇺 Русский

### ✨ Возможности
- 📡 **250+ классических кодов** (NEC, Sony, RC5 и др.) – ваша собственная база.
- 🌍 **Полная база TV-B-Gone: NA (135 кодов) и EU (122 кода)** – оригинальные паттерны, извлечённые из Java‑источника.
- 🌐 **Красивый веб‑интерфейс** с анимацией «матричного дождя», индикатором прогресса и отображением текущей команды.
- 🎮 **Режимы**: последовательный / случайный для каждого набора (классика, NA, EU).
- 🛑 **Мгновенная остановка** – кнопка «Стоп» реагирует даже во время передачи.
- 💡 Встроенный светодиод мигает для индикации (двойное мигание при смене режима).
- 📶 **Автономная точка доступа Wi‑Fi** – роутер не нужен.

### 🔧 Необходимое оборудование
- Плата ESP8266 (NodeMCU, Wemos D1 Mini и т.п.)
- Модуль IR‑передатчика (например, KY‑005) **или** дискретные компоненты (ИК‑светодиод + транзистор + резисторы)

### 📥 Установка и использование

#### 1. Получите коды TV-B-Gone (если нужны)
Запустите приложенный Python‑скрипт `extract_tvbg.py` на исходном Java‑файле (`TV-B-Gone_IR_Codes.java`) – будут созданы `tvbg_na.h` и `tvbg_eu.h`.  
Эти файлы содержат все сырые паттерны, размещённые в **PROGMEM**.

#### 2. Подготовьте скетч
- Поместите `tvbg_na.h` и `tvbg_eu.h` в папку со скетчем.
- Откройте `IR_BLASTER.ino` в Arduino IDE.
- **Замените пример массива `powerCodes`** на свои 250+ кодов (или оставьте пример для теста).
- При необходимости измените номера пинов (`IR_LED_PIN` = 5 → D1; `STATUS_LED_PIN` = 2 – встроенный светодиод).

#### 3. Установите библиотеки
- Поддержка плат ESP8266 (через менеджер плат)
- **IRremoteESP8266** (через менеджер библиотек)

#### 4. Загрузите и пользуйтесь
- Выберите свою плату (например, NodeMCU 1.0) и порт, загрузите скетч.
- После запуска подключитесь к Wi‑Fi сети **`IR_Power_250`** (пароль `12345678`).
- Откройте браузер по адресу **`http://192.168.4.1`**.
- Выберите режим:
  - **Classic Sequential / Random** – ваши 250+ кодов.
  - **NA Sequential / Random** – 135 паттернов для Северной Америки.
  - **EU Sequential / Random** – 122 паттерна для Европы.
- Нажмите **STOP** для мгновенной остановки.

### 📁 Структура кода
- `IR_BLASTER.ino` – основной скетч с веб‑сервером, обработкой режимов и функциями отправки ИК.
- `tvbg_na.h`, `tvbg_eu.h` – сгенерированные заголовки со всеми сырыми паттернами в PROGMEM.
- `extract_tvbg.py` – Python‑скрипт для извлечения и преобразования паттернов из исходного Java‑файла.

### ⚙️ Настройка под себя
- **Скорость отправки**: измените `sendIntervalClassic` и `sendIntervalRaw` (в миллисекундах).
- **Добавление своих кодов**: дополните массив `powerCodes` (используйте ту же структуру `IRCode`).
- **Параметры Wi‑Fi**: измените `ap_ssid` и `ap_password` в начале скетча.

### 📜 Лицензия
MIT License – подробности в файле [LICENSE](LICENSE).

### 🙏 Благодарности
- [TV-B-Gone](https://github.com/sambowenh/TV-B-Gone-UK-Edition) за оригинальные ИК‑коды.
- [Библиотека IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266) от crankyoldgit.
- Прошивка Bruce за вдохновение.
