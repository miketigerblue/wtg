| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

# Wheel Tick Generator (WTG) for ZED-F9R

## Overview

The Wheel Tick Generator (WTG) is designed to simulate real locomotive wheel tick signals for testing and development purposes, particularly in applications involving the ZED-F9R module. This component generates synthetic wheel tick signals based on configurable parameters such as frequency and pulse width, making it an essential tool for accurately measuring speed and distance in locomotive applications.

## Features

- Generates synthetic wheel tick signals to simulate real locomotive wheel movements.
- Configurable tick frequency and pulse width for flexible testing scenarios.
- Direct integration with ZED-F9R's opto-isolated inputs for enhanced accuracy and reliability.
- Designed for ESP32 microcontrollers, leveraging the robust and versatile ESP-IDF framework.

## Hardware Requirements

- ESP32 microcontroller board
- Connection cables suitable for your ESP32 board and testing setup
- Optional: Oscilloscope for signal verification

## Software Requirements

- ESP-IDF v5.2 or later
- Basic development tools (e.g., Git, text editor, CMake)

## Setup Instructions

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/miketigerblue/wtc.git
   cd wtc
   ```
