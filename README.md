# ESP8266 IR TV Remote

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

---

## 🇬🇧 English

**ESP8266-based infrared transmitter** with 250+ TV power codes (TV-B-Gone database) and a stylish web interface inspired by "The Matrix" – featuring digital rain animation, real-time progress bar, and instant control.

### ✨ Features

- 📡 **250+ real power codes** from TV-B-Gone (NA + EU regions)
- 🌐 **Beautiful web interface** with:
  - **Matrix "digital rain"** animated background
  - **Progress bar** – shows sequential mode progress
  - **Current command display** – see which TV brand code is being sent
  - **Fully responsive** – works on phones, tablets, desktops
  - **Glowing neon buttons** with hover effects
  - **Instant Stop** – responsive even during transmission
- 🎮 Two modes:
  - **Sequential** – sends all 250 codes one by one (200ms interval, adjustable)
  - **Random** – sends random codes continuously
- 💡 Built-in LED blinks for status (double blink when entering random mode)
- 📶 Standalone **Wi-Fi access point** – no router or internet needed

### 🔧 Hardware Required

- ESP8266 board (NodeMCU, Wemos D1 Mini, etc.)
- IR transmitter options:
  - **Ready-made module** (e.g., KY-005) – easiest, just connect VCC, GND, DATA
  - **Discrete components** – IR LED (940nm) + NPN transistor (BC547/2N2222) + resistors for max range
- Breadboard and jumper wires

### 📋 Wiring Diagram

#### Option 1: Ready-made IR module (e.g., KY-005)

| Module | ESP8266 |
|--------|---------|
| GND    | GND     |
| VCC    | 3.3V (or 5V if available) |
| DATA   | D1 (GPIO5) |

![Module connection](docs/schematic_module.png)

#### Option 2: Transistor circuit (for maximum range)
GPIO5 (D1) ──┬── 220Ω ── base BC547
└── 10kΩ ── GND
Collector ── IR LED anode ── 100Ω ── +3.3V
Emitter ── GND

### 🌐 Web Interface Preview

*Live demo: The background animates with green "digital rain", the progress bar fills as commands are sent, and the current TV brand is displayed in real time.*

### 🚀 Getting Started

#### Prerequisites
- Arduino IDE with ESP8266 board support installed (via Board Manager)
- Library: `IRremoteESP8266` (install via Library Manager)

Select your board (e.g., NodeMCU 1.0) and correct COM port

Upload the sketch

Usage
After uploading, the ESP8266 creates a Wi-Fi network:

SSID: IR_Power_250

Password: 12345678

Connect your phone/laptop to this network

Open a browser and go to http://192.168.4.1

Use the three buttons:

Sequential – sends all 250 codes in order

Random – sends random codes continuously

Stop – halts transmission immediately

📝 Code Structure
ir_tv_remote.ino – main sketch with:

Wi-Fi Access Point setup

Web server with AJAX endpoint /mode

IR transmission logic (supports NEC, Sony SIRC, RC5, RC6, Samsung, Panasonic, JVC, LG, Sharp, Denon)

Non-blocking LED blinking

IRCode structure – stores protocol, address, command, bits, and optional raw data

250+ codes in three sections: old examples + NA region + EU region

⚙️ Customization
Adjust transmission speed by changing sendInterval (default 200ms). For older TVs, increase to 500ms or more. Add your own codes by appending to the powerCodes array.

🤝 Contributing
Feel free to open issues or submit pull requests. If you have additional TV power codes, please share!

RU 

Инфракрасный передатчик на ESP8266 с 250+ кодами выключения телевизоров (база TV-B-Gone) и красивым веб-интерфейсом в стиле «Матрицы» – с анимированным цифровым дождём, индикатором прогресса и мгновенным управлением.

✨ Возможности
📡 250+ реальных кодов из базы TV-B-Gone (регионы NA + EU)

🌐 Великолепный веб-интерфейс:

Фон «Цифровой дождь» – анимированные зелёные символы

Индикатор прогресса – показывает, сколько команд отправлено

Отображение текущей команды – видно, какой код передаётся

Адаптивный дизайн – на телефонах, планшетах и ПК

Неоновые кнопки с эффектами при наведении

Мгновенная остановка – кнопка «Стоп» реагирует даже во время передачи

🎮 Два режима:

Последовательный – отправляет все 250 кодов по очереди (интервал 200 мс, настраивается)

Случайный – непрерывно отправляет случайные коды

💡 Мигание встроенным светодиодом (дважды при входе в случайный режим)

📶 Автономная точка доступа Wi-Fi – роутер не нужен

🔧 Необходимое оборудование
Плата ESP8266 (NodeMCU, Wemos D1 Mini и т.п.)

Варианты ИК-передатчика:

Готовый модуль (например, KY-005) – просто подключите VCC, GND, DATA

Дискретные компоненты – ИК-светодиод (940нм) + транзистор BC547/2N2222 + резисторы для максимальной дальности

Макетная плата и провода

📋 Схема подключения
Вариант 1: Готовый IR-модуль (например, KY-005)
Модуль	ESP8266
GND	GND
VCC	3.3V (или 5V, если есть)
DATA	D1 (GPIO5)
https://docs/schematic_module.png

Вариант 2: Схема с транзистором (максимальная дальность)
text
GPIO5 (D1) ──┬── 220 Ом ── база BC547
             └── 10 кОм ── GND
Коллектор ── ИК-светодиод (анод) ── 100 Ом ── +3.3V
Эмиттер ── GND
https://docs/schematic_transistor.png

Живая демонстрация: фон анимирован зелёным «цифровым дождём», прогресс-бар заполняется, название текущего бренда обновляется в реальном времени.

🚀 Начало работы
Предварительные требования
Arduino IDE с поддержкой ESP8266 (установить через менеджер плат)

Библиотека: IRremoteESP8266 (установить через менеджер библиотек)

Выберите свою плату (например, NodeMCU 1.0) и правильный COM-порт

Загрузите скетч

Использование
После загрузки ESP8266 создаст Wi-Fi сеть:

SSID: IR_Power_250

Пароль: 12345678

Подключитесь к этой сети с телефона или ноутбука

Откройте браузер и перейдите по адресу http://192.168.4.1

Используйте три кнопки:

Sequential – последовательная отправка всех 250 кодов

Random – случайная отправка

Stop – остановка

📝 Структура кода
ir_tv_remote.ino – основной скетч:

Точка доступа Wi-Fi

Веб-сервер с AJAX-обработчиком /mode

Логика ИК-передачи (поддерживаются NEC, Sony SIRC, RC5, RC6, Samsung, Panasonic, JVC, LG, Sharp, Denon)

Неблокирующее мигание светодиодом

Структура IRCode – хранит протокол, адрес, команду, битность и raw-данные

250+ кодов разбиты на три раздела: старые примеры + регион NA + регион EU

⚙️ Настройка
Скорость отправки меняется параметром sendInterval (по умолчанию 200 мс). Для старых телевизоров увеличьте до 500 мс. Свои коды добавляйте в массив powerCodes.

🤝 Участие в разработке
Создавайте issues или pull request'ы. Если у вас есть дополнительные коды выключения телевизоров – поделитесь!
