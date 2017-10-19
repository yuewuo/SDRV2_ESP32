# SDRV2_ESP32 Project

Developed for ESP32 board, this project integrate all useful functions and common settings for SDRV2.

To minimalize loading the firmware, we **strongly** recommend that everything use initial and on-processing configuration but not different firmware for different devices.

## WIFI Setup

Use `make menuconfig` to configure the WIFI SSID and PASSWORD.

## DuHTTP Server

The Idea of DuHTTP (Duplex HTTP Protocol) is raised by us, which provides several interesting features.

1. The ESP32 device works like a HTTP server which allows us to use web browser to control it.
2. And also, ESP32 could send HTTP request to the "browser" when it's long connection.
3. HTTP protocol enables a readable and flexible method to control and debug the ESP32 devices.

## OLED

We support SPI OLED module by just include `lib/lib.h`, which means that it could work with debugging.

## GPIO

Configure GPIO mode by HTTP request.