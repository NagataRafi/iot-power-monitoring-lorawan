// lmic_project_config.h
// Konfigurasi untuk:
// - ESP32 Arduino core 3.x
// - Region AS923-3
// - SX1276 (MRV-RFM95W)
// - MCCI LoRaWAN LMIC library v6.x

// ===== REGION =====
#define CFG_as923 1
#define CFG_as923_subband 3   // AS923-3 (923.2 / 923.4 MHz)

// ===== RADIO =====
#define CFG_sx1276_radio 1

// ===== CLOCK ERROR =====
// Wajib untuk ESP32 — toleransi clock drift antara node dan gateway
#define LMIC_ENABLE_arbitrary_clock_error 1

// ===== INTERRUPT — JANGAN aktifkan ini di ESP32 core 3.x =====
// LMIC_USE_INTERRUPTS menyebabkan Interrupt WDT timeout di ESP32 core 3.x
// Baris berikut sengaja di-comment / tidak diaktifkan:
// #define LMIC_USE_INTERRUPTS

// ===== TIMING =====
// Tambahan toleransi waktu RX window (dalam ms)
// Berguna jika gateway lambat merespons join accept
#define LMIC_CLOCK_ERROR_PPM 10000  // 1% toleransi — aman untuk ESP32

// ===== OPSIONAL — nonaktifkan fitur yang tidak dipakai =====
// Hemat memori dan kurangi kompleksitas ISR
#define DISABLE_PING     1
#define DISABLE_BEACONS  1
