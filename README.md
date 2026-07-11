<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
        <li><a href="#features">Features</a></li>
      </ul>
    </li>
    <li>
      <a href="#architecture">Architecture</a>
      <ul>
        <li><a href="#two-board-split">Two-board split</a></li>
        <li><a href="#dual-core-task-layout-esp32">Dual-core task layout (ESP32)</a></li>
        <li><a href="#safety-watchdog">Safety watchdog</a></li>
        <li><a href="#spi-protocol">SPI protocol</a></li>
      </ul>
    </li>
    <li><a href="#hardware--wiring">Hardware &amp; wiring</a></li>
    <li><a href="#getting-started">Getting Started</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->

## About The Project

Janus Rover is a rover based on the ELEGOO Smart Robot Car Kit V3.0, hugely expanded past the original kit: an ESP32-CAM was mounted on a servo-driven camera mount, movement/temperature/humidity sensors were added, the original Arduino Uno was swapped for an Arduino MEGA 2560 to have enough I/O for everything, and virtually all of the control software is custom.

The rover is split across **two microcontrollers talking over SPI** — an architecture choice, not a limitation, that lets the ESP32 focus on networking/camera work while the MEGA handles real-time motor control and sensors without either one blocking the other.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Built With

- C++ (Arduino framework, ESP-IDF/FreeRTOS underneath on the ESP32 side)
- FreeRTOS (dual-core task scheduling, mutexes, `std::atomic` for cross-core shared state)
- HTML / CSS / JavaScript (control panel served directly from the ESP32)
- SPI (inter-board communication), SMTP (motion-triggered email alerts)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Features

- Steer the rover from a browser or a physical/web joystick
- Live MJPEG video stream from the ESP32-CAM, viewable in the control panel
- PIR motion detection on the MEGA triggers a snapshot + email alert from the ESP32
- Temperature/humidity telemetry, forwarded from the MEGA to the ESP32 and on to a companion web server
- Low-energy mode (reduced WiFi power state) toggle from the UI
- A rolling log of the last movements (`MoveLog`), queryable from the control panel

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ARCHITECTURE -->

## Architecture

### Two-board split

| Board                      | Role                  | Handles                                                                    |
| -------------------------- | --------------------- | -------------------------------------------------------------------------- |
| **ESP32-CAM** (AI-Thinker) | Network-facing "head" | WiFi, web server + control panel, MJPEG streaming, SMTP alerts, SPI master |
| **Arduino MEGA 2560**      | Real-time "spine"     | Motor driving, PIR/DHT/ultrasonic sensors, SPI slave                       |

The two boards never share memory — everything crosses the SPI link as short text messages (see [SPI protocol](#spi-protocol) below).

### Dual-core task layout (ESP32)

The ESP32-CAM is a dual-core chip, and the sketch deliberately spreads work across both cores so a slow HTTP client can never stall the camera stream or the SPI heartbeat:

| Task                         | Core | Responsibility                                                                             |
| ---------------------------- | ---- | ------------------------------------------------------------------------------------------ |
| `loop()` (Arduino main task) | 1    | Pushes MJPEG frames, sends the `"alv"` heartbeat every 700ms                               |
| `handleSPIRequests`          | 1    | Polls the MEGA over SPI, reacts to motion/telemetry messages                               |
| `ResponseToClientRequests`   | 0    | Serves the web control panel and HTTP API, kept off core 1 so it can never delay streaming |

Cross-core shared state is protected two ways depending on shape: simple scalars (`connected`, telemetry readings, upload flags) use `std::atomic`; the SPI bus itself is guarded by a FreeRTOS mutex (`spiMutex`) since two tasks could otherwise start a transaction on the same physical bus at once.

### Safety watchdog

If the ESP32 ever stops responding (crash, WiFi issue, reset), the MEGA needs to stop the motors on its own rather than trust a signal that may never arrive. This is done with a heartbeat, not a static "alive" pin: the ESP32 sends `"alv"` over SPI every 700ms; the MEGA resets a timer on every heartbeat it receives, and if 2.2 seconds pass without one, it force-stops both drive motors. As long as the link is alive, driving is uninterrupted; if it isn't, the rover fails safe.

### SPI protocol

Messages are short, human-readable strings sent over a shared SPI link (ESP32 = master, MEGA = slave, 4 MHz clock — chosen because the MEGA's AVR core caps reliable SPI-slave operation at `f_osc / 4`, i.e. 16 MHz / 4). A few examples:

| Message                         | Direction    | Meaning                                          |
| ------------------------------- | ------------ | ------------------------------------------------ |
| `alv`                           | ESP32 → MEGA | Heartbeat / still-alive                          |
| `MOVE`                          | MEGA → ESP32 | PIR sensor triggered                             |
| `_t{temp}h{humidity}v{voltage}` | MEGA → ESP32 | Telemetry                                        |
| `sendData`                      | ESP32 → MEGA | Manual snapshot/telemetry trigger from the UI    |
| `lowEn` / `norEn`               | ESP32 → MEGA | Energy mode changed                              |
| `/prec1`…`/uprec4`              | ESP32 → MEGA | Precision movement commands (varying stop delay) |

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- HARDWARE -->

## Hardware & wiring

Powered by two Li-Ion 18650 3.7V cells. Key pin assignments (see the `.ino` files for the full list):

**ESP32-CAM**

| Signal                      | Pin                       |
| --------------------------- | ------------------------- |
| SPI SCLK / MISO / MOSI / SS | 14 / 12 / 13 / 15         |
| Camera (AI-Thinker pinout)  | see `rover_web_board.ino` |

**Arduino MEGA 2560**

| Signal                      | Pin            |
| --------------------------- | -------------- |
| SPI SCK / SS                | 52 / 53        |
| Motor enable (left / right) | 5 / 6          |
| Motor direction IN1–IN4     | 7 / 8 / 9 / 11 |
| Ultrasonic TRIG / ECHO      | A5 / A4        |
| PIR sensor                  | 40             |
| DHT (temp/humidity)         | 22             |
| Camera servo                | 3              |

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->

## Getting Started

1. Flash `rover_base_board/rover_base_board.ino` to the Arduino MEGA 2560 (standard Arduino IDE, no extra config needed).
2. Copy `rover_web_board/wifi_config.h.example` to `rover_web_board/wifi_config.h` and fill in your WiFi credentials, server IP and SMTP (Gmail app password) details — this file is gitignored on purpose, never commit real credentials.
3. In Arduino IDE, select an ESP32 board profile with a large enough app partition (**Huge APP (3MB No OTA/1MB SPIFFS)** under _Tools → Partition Scheme_) — the sketch won't fit in the default scheme.
4. Flash `rover_web_board/rover_web_board.ino` to the ESP32-CAM.
5. Wire the two boards' SPI pins together (see [Hardware & wiring](#hardware--wiring)) plus a common ground.
6. Power up the MEGA first, then the ESP32 — check the Serial monitor for the assigned IP, then open it in a browser.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->

## License

Copyright 2026 Patryk "MrMayestic" Pilch

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

<p align="right">(<a href="#readme-top">back to top</a>)</p>
