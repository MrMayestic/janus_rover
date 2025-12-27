<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li><a href="#overview">Overview</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->

## About The Project

Janus rover is a rover based on ELEGOO SMART ROBOT CAR KIT V3.0, but was hugely expanded. I added ESP32-cam board on special attachment on servo motor, added movement, temperature and humidity sensors, changed original Arduino Uno board to Arduino MEGA 2560 and most important, new code.

Rover can:

- Be steered via web browser or special joystick (also online)
- Detect motion and send email with photos
- Stream video via ESP32 camera
- Measure temperature and humidity of air and send it to website

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Built With

- C++
- Arduino IDE
- HTML
- CSS
- JavaScript

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- OVERVIEW -->

## Overview

Rover is using Ardunio MEGA 2560 borad and ESP32-cam with ESP32-S dual-core CPU. It is build on ELEGOO chasis with added sensors. ESP32 board's code needs additional wifi_config.h file with Wi-Fi and email credentials. Rover is powered by two Li-Ion 18650 3.7V batteries.

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
