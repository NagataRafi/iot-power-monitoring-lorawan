# Konfigurasi Hardware Lengkap

Panduan detail perakitan, pinout, dan konfigurasi hardware untuk sistem monitoring gardu listrik.

## Bill of Materials (BOM)

| Qty | Komponen | Part Number | Spesifikasi | Catatan |
|-----|----------|-------------|-------------|---------|
| 1 | Microcontroller | ESP32-WROOM-DA | 38-pin DevKit V1, 4MB Flash | |
| 3 | Sensor Daya | PZEM-004T v3.0 | 100A / 240V AC, Modbus RTU | Hitam atau Hijau |
| 1 | Modul LoRa | Marnov MRV-RFM95W | SX1276, +20dBm, 923MHz | |
| 1 | Antena LoRa | 923MHz | SMA connector, 5-8dBi | |
| 2 | Kapasitor | 100µF / 16V | Electrolytic | Untuk decoupling 3.3V |
| 1 | Resistor | 10kΩ | 1/4W | Pull-up DIO0 (optional) |
| 1 | Power Bank | 4200mAh 3.7V | Li-ion | Untuk testing portabel |
| 1 | USB Cable | Micro USB | 5V power input | |
| 3 | Terminal Block | 2P 5.08mm | Untuk PZEM input | |
| 1 | Jumper Wire | Dupont Female-Female | Untuk breadboard | |
| 1 | Breadboard | 830 hole | Untuk prototyping | |

### Gateway Hardware

| Qty | Komponen | Part Number | Spesifikasi |
|-----|----------|-------------|-------------|
| 1 | SBC | Raspberry Pi 4 Model B | 4GB RAM, Broadcom BCM2711 |
| 1 | Concentrator | Seeed Studio WM1302 | SX1302 + SX1250, SPI mode |
| 1 | Antena | 923MHz SMA | 5-8dBi |
| 1 | Power Supply | USB-C 5V/3A | Untuk Pi |
| 1 | MicroSD Card | 32GB+ | Class 10, UHS-I |
| 1 | HAT Connector | 40-pin | Untuk WM1302 ke Pi |
| 1 | Case | Raspberry Pi 4 Case | Opsional, untuk protection |

## Wiring Diagram Node (ESP32)

### ESP32 to PZEM Split UART

```
ESP32 UART2 (GPIO16/17)
  |
  +---- RX (GPIO16) ---- TX (PZEM-001, PZEM-002)
  |
  +---- TX (GPIO17) ---- RX (PZEM-001, PZEM-002)
  |
  +---- GND ----------- GND (PZEM-001, PZEM-002)

ESP32 UART1 (GPIO21/22)
  |
  +---- RX (GPIO21) ---- TX (PZEM-003)
  |
  +---- TX (GPIO22) ---- RX (PZEM-003)
  |
  +---- GND ----------- GND (PZEM-003)

ESP32 3.3V (NOT 5V) ---- 5V (PZEM-001, PZEM-002, PZEM-003)
```

### ESP32 to MRV-RFM95W SPI

```
ESP32 GPIO23 (MOSI)  ---- DIN (RFM95W)
ESP32 GPIO19 (MISO)  ---- DOUT (RFM95W)
ESP32 GPIO18 (CLK)   ---- CLK (RFM95W)
ESP32 GPIO5 (CS)     ---- NSS (RFM95W)
ESP32 GPIO14 (RST)   ---- RST (RFM95W)
ESP32 GPIO4 (INT)    ---- DIO0 (RFM95W)
ESP32 GND            ---- GND (RFM95W)
ESP32 3.3V           ---- VCC (RFM95W)
```

### Decoupling Capacitor Placement

```
ESP32 3.3V Rail:

    -----[100µF C1]-----+-----[100µF C2]-----
                        |
                    (Near RFM95W)
                        |
                      RFM95W VCC
```

Tempatkan C1 dekat pin 3.3V ESP32, C2 dekat VCC RFM95W untuk stabilitas.

## Pinout Detail ESP32-WROOM-DA

```
ESP32-WROOM-DA 38-pin Pinout:

                 +---------+
            EN --+ 1    38 +-- GND
            IO36+-- 2    37 +-- IO13 (SPI CLK - alternative)
            IO37+-- 3    36 +-- IO12 (STRAPPING - avoid)
            IO38+-- 4    35 +-- IO27 (alternative IO)
            IO39+-- 5    34 +-- IO26 (alternative IO)
            IO34+-- 6    33 +-- IO25 (alternative IO)
            IO35+-- 7    32 +-- IO33 (alternative IO)
            GND --+ 8    31 +-- IO32 (alternative IO)
            IO22+-- 9    30 +-- IO31 (alternative IO)
            IO21+--10    29 +-- IO30 (alternative IO)
            3.3V+--11    28 +-- GND
            GND --+12    27 +-- IO29 (input only)
            IO19+--13    26 +-- IO24 (input only)
            IO18+--14    25 +-- IO23
            IO17+--15    24 +-- IO22 (UART1 TX)
            IO16+--16    23 +-- IO21 (UART1 RX)
            5V  +--17    22 +-- IO20 (input only)
            GND --+18    21 +-- IO15
            IO14+--19    20 +-- IO14 (LoRa RST)
            IO13+--20    19 +-- IO13 (SPI CLK)
            IO12+--21    18 +-- IO12 (STRAPPING - avoid)
            IO11+--22    17 +-- IO11
            IO10+--23    16 +-- IO10
            IO9 +--24    15 +-- IO9
            IO8 +--25    14 +-- IO8
            GND --+26    13 +-- GND
            3.3V+--27    12 +-- IO7 (SDIO)
            GND --+28    11 +-- IO6 (SDIO)
            IO5 +--29    10 +-- IO5 (SPI CS)
            IO6 +--30     9 +-- IO4 (LoRa DIO0 - interrupt)
            IO4 +--31     8 +-- IO3 (UART0 RX)
            IO3 +--32     7 +-- IO2 (alternative DIO0)
            IO2 +--33     6 +-- IO1 (UART0 TX)
            IO1 +--34     5 +-- IO0 (BOOT)
            GND --+35     4 +-- GND
            GND --+36     3 +-- GND
                  +37, 38 +-- Extra pads (solder)
                 +---------+

Key pins untuk project ini:
- GPIO23 (pin 25): MOSI (LoRa)
- GPIO19 (pin 13): MISO (LoRa)
- GPIO18 (pin 14): CLK (LoRa)
- GPIO5 (pin 29): CS (LoRa)
- GPIO14 (pin 19): RST (LoRa)
- GPIO4 (pin 31): DIO0 (LoRa - INTERRUPT CAPABLE)
- GPIO16 (pin 16): RX UART2 (PZEM)
- GPIO17 (pin 15): TX UART2 (PZEM)
- GPIO21 (pin 10): RX UART1 (PZEM)
- GPIO22 (pin 9): TX UART1 (PZEM)

HINDARI:
- GPIO34 (pin 6): Input only
- GPIO12 (pin 21): Strapping pin
- GPIO0 (pin 34): Boot strapping pin
```

## Petunjuk Konfigurasi Sensor PZEM-004T

### Setting Modbus Address (Sekali saja, di awal)

PZEM-004T memiliki saklar fisik atau menu untuk setting address. Setiap sensor harus unik:

1. Sensor Fasa R: Address = 0x01
2. Sensor Fasa S: Address = 0x02
3. Sensor Fasa T: Address = 0x03

Metode 1: Saklar fisik (jika tersedia)
- Buka casing PZEM
- Cari switch DIP untuk Modbus address
- Set sesuai tabel address di produk

Metode 2: AT Command (via serial terminal)
```
AT+ADDR=01   # Set address 0x01
AT+ADDR=02   # Set address 0x02
AT+ADDR=03   # Set address 0x03
```

Metode 3: Firmware reset (reset ke broadcast 0xF8)
```cpp
PZEM004Tv30 pzem(Serial2, 0xF8);  // Broadcast address
pzem.setAddress(0x01);              // Set new address
```

### Verifikasi Sensor

Test konektivitas sebelum deploy:

```cpp
#include <PZEM004Tv30.h>

PZEM004Tv30 pzem1(Serial2, 0x01);  // GPIO16/17 RX/TX

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Testing PZEM at address 0x01...");
  
  float v = pzem1.voltage();
  if (isnan(v)) {
    Serial.println("FAIL - No response from PZEM");
  } else {
    Serial.print("OK - Voltage: ");
    Serial.println(v);
  }
}

void loop() {
  delay(1000);
}
```

Jika response: voltage = NaN (not a number), berarti:
- Address mismatch
- Wiring incorrect
- Baud rate 9600 not matching
- Sensor power off

### Spesifikasi Teknis PZEM-004T

```
Electrical Ratings:
- Rated Voltage: 240V AC
- Rated Current: 100A AC
- Rated Power: 24kW
- Frequency: 45-65 Hz (standard) atau 50/60 Hz (selektif)

Measurement Accuracy:
- Voltage: 0.5% ± 1 digit
- Current: 1% ± 1 digit
- Power: 2% ± 1 digit
- Energy: 2% ± 4 digit
- Frequency: 0.1% (typical)
- Power Factor: 0.01 (typical)

Interface:
- Modbus RTU (master-slave)
- Baud Rate: 9600 bps (default)
- Data Frame: 8 bit data, 1 stop bit, Even parity
- Address range: 0x01 - 0xF7 (0xF8 = broadcast)

Communication Parameters:
- Response time: typically <100ms
- Holding registers for data reading
- No external memory required

Power Supply:
- Teknik: From AC line (CT for current measurement)
- Voltage input: Via AC voltage transformer
- Internal circuit: Capacitive coupling for isolation
```

## Perakitan Fisik

### Langkah 1: Persiapan Breadboard

```
Breadboard 830-hole layout:

Top rails (+5V, GND):
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| + | + | + | + | + | + | + | + | + | + | + | + | + | + | + |
| - | - | - | - | - | - | - | - | - | - | - | - | - | - | - |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

Ground rails (pada dua sisi untuk kemudahan wiring)
```

### Langkah 2: Penempatan ESP32

1. Letakkan ESP32 di tengah breadboard (meninggalkan ruang di kiri-kanan)
2. Hubungkan:
   - Pin GND ESP32 ke ground rail breadboard
   - Pin 3.3V ESP32 ke +3.3V rail (jika breadboard punya)
   - Atau: Gunakan USB power untuk 5V, regulator untuk 3.3V

### Langkah 3: Penempatan RFM95W

1. Tempatkan RFM95W ke breadboard dengan SPI line ke ESP32
2. Jarak dekat untuk minimal noise pada SPI
3. Tambahkan kapasitor 100µF dekat pin VCC

### Langkah 4: Koneksi Sensor PZEM

1. Serial2 (GPIO16/17):
   - Connect PZEM-001 TX ke GPIO16 (RX)
   - Connect PZEM-001 RX ke GPIO17 (TX)
   - Connect PZEM-002 ke line yang sama (GPIO16/17)
   - Common GND

2. Serial1 (GPIO21/22):
   - Connect PZEM-003 TX ke GPIO21 (RX)
   - Connect PZEM-003 RX ke GPIO22 (TX)
   - Common GND

3. Power:
   - PZEM-001, 002, 003 masing-masing 5V input
   - Source: dari 3.3V ESP32 (dalam kasus ini stabil)
   - Common GND dari ESP32

### Langkah 5: Testing

```bash
1. Connect ESP32 to computer via USB
2. Open Arduino IDE Serial Monitor (115200 baud)
3. Upload test code
4. Verify output:
   - PZEM readings (voltage, current, power)
   - LoRa transmission status
   - Frame count
```

## Gateway Hardware Setup (Raspberry Pi)

### Koneksi WM1302 ke Raspberry Pi

```
WM1302 Pin          Raspberry Pi GPIO
+-----+
| 1   +---- 3.3V        Pin 1 (3.3V)
| 2   +---- GND         Pin 6 (GND)
| 3   +---- SPI MOSI    Pin 19 (GPIO10 MOSI)
| 4   +---- SPI MISO    Pin 21 (GPIO9 MISO)
| 5   +---- SPI CLK     Pin 23 (GPIO11 CLK)
| 6   +---- SPI CS      Pin 24 (GPIO8 CE0)
| 7   +---- RESET       Pin 22 (GPIO25)
| 8   +---- GPS TX      (unused, Pin 8 GPIO14)
| 9   +---- GPS RX      (unused, Pin 10 GPIO15)
|10   +---- GND         Pin 9 (GND)
+-----+

Alternatif SPI chip select:
- CE0: Pin 24 (GPIO8)
- CE1: Pin 26 (GPIO7)
```

### Enable SPI di Raspberry Pi

```bash
sudo raspi-config
# Navigation:
# 3 Interface Options
# P4 SPI
# <Yes> to enable
# <Ok>
# <Finish> dan reboot
```

Verifikasi:
```bash
ls -l /dev/spi*
# Output: /dev/spidev0.0, /dev/spidev0.1 (harus ada)
```

### Penempatan Fisik

1. Antena LoRa:
   - Jauh dari elektronik noisy
   - Outdoor atau jendela untuk jangkauan maksimal
   - Height di atas 2m lebih baik

2. Raspberry Pi:
   - Letakkan di lokasi aman dengan ventilasi baik
   - Dekat dengan AC power
   - Akses ke Ethernet/Wi-Fi

3. WM1302:
   - Mount via GPIO HAT connector (40-pin)
   - Atau hardwire via jumper cables untuk testing
   - Case protektif opsional

## Troubleshooting Hardware

### ESP32 tidak detected

```
Penyebab:
1. USB cable rusak / data line terputus
2. UART driver tidak installed (CH340 / CP2102)
3. Microcontroller corrupt

Solusi:
- Ganti USB cable dengan USB 2.0 (bukan USB 3.0)
- Install CH340 driver dari internet
- Jika masih fail, gunakan FTDI programmer untuk recover
```

### PZEM tidak respond

```
Penyebab:
1. Address mismatch dengan kode
2. Pin TX/RX salah / terbalik
3. Sensor power off
4. Baud rate 9600 tidak match

Solusi:
- Double check address di kode vs fisik di sensor
- Reverse TX/RX dan test
- Cek voltage dengan multimeter
- Confirm baud rate 9600 dengan AT command
```

### LoRa tidak transmit

```
Penyebab:
1. DIO0 bukan interrupt-capable pin
2. LMIC_USE_INTERRUPTS = 1 (harus 0 untuk ESP32)
3. Antenna disconnected
4. Pin CS tidak correct
5. ChirpStack gateway tidak online

Solusi:
- Gunakan GPIO4 atau GPIO2 untuk DIO0
- Disable LMIC interrupts
- Test antenna connection fisik
- Verify CS pin match dengan kode
- Check ChirpStack service status
```

### Gateway WM1302 tidak detect

```
Penyebab:
1. SPI tidak enable
2. Pin connection salah
3. Concentrator power insufficient

Solusi:
- Enable SPI via raspi-config
- Verify GPIO dengan dmesg saat boot
- Gunakan USB power 5A untuk Raspberry Pi
```

## Verifikasi Akhir

Sebelum deployment ke gardu listrik:

- [ ] ESP32 upload berhasil, serial monitor output normal
- [ ] PZEM-001, 002, 003 masing-masing respond (test individual)
- [ ] LoRa join OTAA successful (cek ChirpStack console)
- [ ] Payload decoder di ChirpStack decode correctly
- [ ] Node dan Gateway jarak minimal 100m uplink test 100% PDR
- [ ] Grafana dashboard menerima data real-time
- [ ] Telegram notification working pada threshold
- [ ] Battery power test minimal 8 jam continuous
- [ ] Physical enclosure protection dari cuaca (untuk outdoor)

---

Untuk pertanyaan detail tentang hardware, silakan merujuk pada datasheet komponen di folder documentation/
