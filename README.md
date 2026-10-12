# 🌱 7-in-1 Soil RS485 Integrated Sensor (NPK + pH + EC + Temp + Humidity) - Tokoasia Guide

[![Tokoasia Verified](https://img.shields.io/badge/Tokoasia-Verified-brightgreen?style=flat-square)](https://github.com/tokoasia)
[![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20Arduino%20Uno%20%7C%20Python-blue?style=flat-square)](https://github.com/tokoasia)
[![License](https://img.shields.io/badge/License-MIT-green.svg?style=flat-square)](LICENSE)

Repositori resmi untuk sensor tanah terintegrasi 7-in-1 RS485 Modbus RTU dari **Tokoasia** (SKU: ID1401 / ID1402). Mengukur Nitrogen (N), Fosfor (P), Kalium (K), pH tanah, Salinitas/EC, Temperatur, dan Kelembaban secara simultan.

---

## 📋 Spesifikasi
- **Protokol:** Modbus RTU (RS485)
- **Baud Rate Default:** 4800 bps / 9600 bps (8-N-1)
- **Tegangan Kerja:** DC 9V - 24V
- **Slave ID Default:** `0x01`
- **Parameter Terukur:** Kelembaban, Suhu, Konduktivitas (EC), pH, Nitrogen (N), Fosfor (P), Kalium (K)

---

## 🔌 Skema Wiring (MAX485 / RS485 Shield ke ESP32 / Arduino Uno)
| Kabel Sensor | Modul MAX485 | Keterangan |
| :--- | :--- | :--- |
| **Cokelat (VCC)** | 9V - 24V DC Eksternal | Daya Sensor |
| **Hitam (GND)** | GND Eksternal & GND MCU | Ground Bersama (Common Ground) |
| **Kuning (A+)** | Pin A Modul MAX485 | RS485 Data + |
| **Biru (B-)** | Pin B Modul MAX485 | RS485 Data - |

---

## 💻 Isi Repositori
- `esp32/npk7in1_esp32.ino`: Contoh pembacaan dengan ESP32 Serial2 Hardware UART & ModbusMaster.
- `arduino/npk7in1_uno.ino`: Contoh pembacaan dengan SoftwareSerial Arduino Uno.
- `python/7in1npk.py`: Skrip pembacaan via USB-to-RS485 di PC / Raspberry Pi.

---

## 🛒 Pembelian Resmi
- Beli sensor original di [Shopee Tokoasia](https://shopee.co.id/tokoasia)
- Web Demo & Simulator: [store.tokoasia.me](https://store.tokoasia.me)
