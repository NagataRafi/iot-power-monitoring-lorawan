# Index Dokumentasi & Panduan Publikasi GitHub

Panduan lengkap menggunakan semua dokumentasi untuk publikasi project IoT monitoring gardu listrik di GitHub.

## Struktur Repository yang Direkomendasikan

```
iot-power-monitoring-lorawan/
├── README.md                          # Entrypoint - baca ini dulu!
├── HARDWARE_CONFIG.md                 # Panduan hardware detail
├── SOFTWARE_SETUP.md                  # Panduan software detail
├── TROUBLESHOOTING.md                 # Troubleshooting guide
├── BOM_AND_TESTING.md                 # Bill of Materials & test results
├── DOCUMENTATION_INDEX.md             # File ini
├── LICENSE                            # MIT License
├── .gitignore                         # Exclude files dari git
│
├── firmware/                          # Firmware dan source code
│   ├── esp32_node/
│   │   ├── main.ino                   # Main firmware sketch
│   │   ├── lmic_config.h              # LMIC pin configuration
│   │   ├── payload_encoder.h          # Payload encoding logic
│   │   ├── pzem_handler.h             # PZEM sensor handler
│   │   ├── config.h                   # Project configuration constants
│   │   └── README.md                  # Arduino setup guide
│   │
│   ├── libraries/
│   │   ├── MCCI_LoRaWAN_LMIC/         # MCCI LMIC v6.0.1 (reference)
│   │   └── PZEM004Tv30/               # PZEM library (reference)
│   │
│   └── tests/
│       ├── test_pzem_individual.ino   # Test PZEM satu-satu
│       ├── test_lora_join.ino         # Test LoRaWAN join only
│       └── test_payload_format.ino    # Test payload encoding
│
├── gateway/                           # Raspberry Pi gateway configuration
│   ├── setup_scripts/
│   │   ├── 00_prepare_system.sh       # Update & dependencies
│   │   ├── 01_install_chirpstack.sh   # ChirpStack installation
│   │   ├── 02_install_mqtt.sh         # Mosquitto MQTT setup
│   │   ├── 03_install_influxdb.sh     # InfluxDB setup
│   │   ├── 04_install_grafana.sh      # Grafana setup
│   │   ├── 05_install_nodered.sh      # Node-RED setup
│   │   ├── 06_install_lora_pkt_fwd.sh # Packet forwarder setup
│   │   └── setup_all.sh               # Master script (run all)
│   │
│   ├── config/
│   │   ├── chirpstack.toml            # ChirpStack configuration
│   │   ├── chirpstack-gateway-bridge.toml  # Gateway bridge config
│   │   ├── global_conf.json           # Packet forwarder config
│   │   ├── mosquitto.conf             # MQTT broker config
│   │   ├── influxdb.conf              # InfluxDB configuration
│   │   └── nodered_flows.json         # Node-RED flow export
│   │
│   ├── systemd/
│   │   ├── lora-pkt-fwd.service       # Packet forwarder service
│   │   └── startup_gateway.sh         # Startup automation script
│   │
│   └── README.md                      # Gateway setup quick reference
│
├── dashboard/                         # Grafana & visualization
│   ├── grafana/
│   │   ├── dashboard_aggregate.json   # Total parameters dashboard
│   │   ├── dashboard_per_phase.json   # Per-phase dashboard
│   │   └── dashboard_realtime.json    # Real-time monitoring
│   │
│   ├── influxdb/
│   │   ├── create_database.sh         # Database initialization
│   │   └── retention_policy.sql       # Retention setup
│   │
│   └── README.md                      # Dashboard setup guide
│
├── tests/                             # Testing documentation & results
│   ├── accuracy_test.xlsx             # PZEM accuracy results
│   ├── lora_range_test.xlsx           # LoRa PDR test results
│   ├── latency_analysis.xlsx          # Latency measurements
│   ├── power_consumption.xlsx         # Power profile data
│   └── test_procedures.md             # How to run tests yourself
│
├── documentation/                     # Supporting documents
│   ├── datasheets/
│   │   ├── ESP32-WROOM-DA_datasheet.pdf
│   │   ├── PZEM-004T_datasheet.pdf
│   │   ├── MRV-RFM95W_SX1276_datasheet.pdf
│   │   └── WM1302_datasheet.pdf
│   │
│   ├── architecture/
│   │   ├── system_architecture.png    # System diagram
│   │   ├── wiring_diagram.png         # Hardware connections
│   │   ├── data_flow.png              # Data pipeline flow
│   │   └── network_topology.png       # Network layout
│   │
│   ├── manual/
│   │   ├── pzem_modbus_protocol.pdf   # PZEM protocol reference
│   │   ├── chirpstack_manual.pdf      # ChirpStack documentation
│   │   └── grafana_guide.pdf          # Grafana visualization guide
│   │
│   └── research/
│       ├── literature_review.md       # Background research
│       └── related_work.md            # Similar projects reference
│
├── examples/                          # Example implementations
│   ├── mqtt_client_python.py          # MQTT subscriber example
│   ├── influxdb_query_example.py      # InfluxDB query example
│   ├── telegram_bot_setup.md          # Telegram notification setup
│   └── google_sheets_integration.md   # Google Sheets logging setup
│
└── .github/                           # GitHub specific files
    ├── workflows/
    │   └── ci_tests.yml               # CI/CD workflow (optional)
    │
    ├── ISSUE_TEMPLATE/
    │   ├── bug_report.md              # Bug report template
    │   └── feature_request.md         # Feature request template
    │
    └── pull_request_template.md       # PR submission template
```

## Panduan Membaca Dokumentasi

### 1. Untuk Pemula (Belum Memiliki Hardware)

**Urutan Baca:**
1. **README.md** - Overview lengkap project
2. **BOM_AND_TESTING.md** (Bagian A & D) - Daftar komponen & harga
3. **HARDWARE_CONFIG.md** - Pahami koneksi hardware sebelum beli
4. **SOFTWARE_SETUP.md** (Bagian 1) - Mengerti flow instalasi

**Tujuan:** Memahami project secara menyeluruh sebelum membeli hardware

### 2. Untuk Builder (Sudah Memiliki Hardware)

**Urutan Implementasi:**

**Phase 1: Hardware Assembly (1-2 hari)**
1. HARDWARE_CONFIG.md (Bagian II-IV) - Perakitan breadboard
2. HARDWARE_CONFIG.md (Bagian V-VI) - PZEM konfigurasi & verifikasi
3. Jalankan test firmware di `firmware/tests/`

**Phase 2: Firmware Development (2-3 hari)**
1. firmware/esp32_node/README.md - Arduino setup
2. Copy main.ino ke Arduino IDE
3. Upload dan verifikasi via serial monitor
4. TROUBLESHOOTING.md (Bagian 1-4) jika ada issue

**Phase 3: Gateway Setup (2-3 hari)**
1. SOFTWARE_SETUP.md (Bagian 1-7) - Install semua service
2. gateway/setup_scripts/ - Run automated setup
3. gateway/config/ - Sesuaikan konfigurasi dengan setup Anda
4. Verify services berjalan (SOFTWARE_SETUP.md akhir)

**Phase 4: Testing & Validation (1-2 hari)**
1. BOM_AND_TESTING.md (Bagian C - acuan testing)
2. Jalankan test dengan prosedur yang sama
3. Bandingkan hasil dengan expected values

**Phase 5: Deployment (1 hari)**
1. DEPLOYMENT_CHECKLIST.md
2. Siapkan lokasi & verifikasi akses
3. Deploy dan monitor 24+ jam

**Total: ~1-2 minggu untuk dari 0 hingga operational**

### 3. Untuk Troubleshooting

**Ketika Mengalami Problem:**

1. Cari error message di TROUBLESHOOTING.md table of contents
2. Baca penyebab umum & solusi
3. Ikuti step-by-step troubleshooting
4. Jika tidak terselesaikan → lihat "Getting Help" section

**Contoh:**
- ESP32 tidak detect → TROUBLESHOOTING.md Bagian 1
- LoRa join failed → TROUBLESHOOTING.md Bagian 5
- Data tidak ke Grafana → TROUBLESHOOTING.md Bagian 9

### 4. Untuk Maintenance & Operation

**Rutin Harian:**
- Check dashboard Grafana untuk anomali
- Monitor uptime & latency (TROUBLESHOOTING.md #16 tips)

**Rutin Mingguan:**
- Verify service status (lihat startup script)
- Backup database & konfigurasi

**Rutin Bulanan:**
- Review test results & accuracy
- Update firmware jika ada fix
- Clean logs (lihat TROUBLESHOOTING.md #14)

## File-file Penting & Kegunaan

### Untuk Setup Awal

| File | Fungsi | Prioritas |
|------|--------|-----------|
| README.md | Overview project | ⭐⭐⭐ CRITICAL |
| HARDWARE_CONFIG.md | Hardware setup | ⭐⭐⭐ CRITICAL |
| SOFTWARE_SETUP.md | Gateway software | ⭐⭐⭐ CRITICAL |
| firmware/esp32_node/main.ino | Firmware source | ⭐⭐⭐ CRITICAL |
| gateway/setup_scripts/setup_all.sh | Auto setup | ⭐⭐ Important |

### Untuk Operasional

| File | Fungsi | Prioritas |
|------|--------|-----------|
| TROUBLESHOOTING.md | Problem solving | ⭐⭐⭐ High |
| BOM_AND_TESTING.md | Reference values | ⭐⭐ Medium |
| dashboard/README.md | Dashboard usage | ⭐⭐ Medium |
| examples/ | Integration guides | ⭐ Low |

### Untuk Reference Teknis

| File | Fungsi | Prioritas |
|------|--------|-----------|
| documentation/datasheets/ | Hardware spec | ⭐⭐ Medium |
| documentation/architecture/ | System design | ⭐⭐ Medium |
| test_procedures.md | Testing reference | ⭐ Low |

## Quick Start Command (untuk yang impatient)

Untuk user yang ingin langsung mulai, berikut command cepat:

### ESP32 Setup (5 menit)

```bash
# 1. Download firmware
git clone [repo-url]
cd iot-power-monitoring-lorawan

# 2. Open di Arduino IDE
# File > Open > firmware/esp32_node/main.ino

# 3. Configure (Sketch > Include Library > Manage Libraries)
# Install: MCCI LoRaWAN LMIC v6.0.1 + PZEM004Tv30

# 4. Edit pin & credentials di main.ino

# 5. Upload!
# Ctrl+U
```

### Gateway Setup (30 menit dengan script)

```bash
# 1. SSH ke Raspberry Pi
ssh pi@192.168.100.55

# 2. Clone repo & run setup
git clone [repo-url]
cd iot-power-monitoring-lorawan/gateway
sudo bash setup_scripts/setup_all.sh

# 3. Tunggu ~15 menit (auto setup)

# 4. Verify
bash verify_services.sh

# Done! Access:
# ChirpStack: http://192.168.100.55:8080
# Grafana: http://192.168.100.55:3000
```

## How to Contribute

Jika Anda menemukan bug atau improvement:

1. Buat GitHub Issue dengan template (`.github/ISSUE_TEMPLATE/`)
2. Jelaskan problem, reproduction steps, expected vs actual
3. Attach logs & diagnostic info
4. Untuk fix: Fork → branch → commit → PR

Lihat CONTRIBUTING.md (jika ada) untuk detail.

## License & Attribution

Project ini menggunakan MIT License. Artinya:
- Bebas digunakan untuk tujuan komersial & non-komersial
- Harus menyertakan license file dalam distribusi
- Tanpa warranty
- Wajib mention original authors

**Original Authors:**
- Muhammad Rafi Nagata Afendi (2331130049)
- Thariza Revania Anggani (2331130043)

Pembimbing:
- Ginanjar Suwasono Adi, S.ST., M.Sc.
- Dodit Suprianto, S.KOM., M.T.

Politeknik Negeri Malang, 2026

## FAQ - Frequently Asked Questions

### Q: Berapa lama setup keseluruhan?
A: 1-2 minggu dari 0 hingga operational, tergantung pengalaman hardware

### Q: Apa hardware minimum yang dibutuhkan?
A: Lihat BOM_AND_TESTING.md - ~4.7 juta IDR untuk testing setup

### Q: Bisakah dimodifikasi untuk gardu listrik berbeda?
A: Dapat! Lihat firmware/esp32_node/config.h untuk customize

### Q: Apakah support for regions selain AS923?
A: Yes, LMIC support semua region. Edit `lmic_util.c` region define

### Q: Bagaimana untuk scale ke banyak gardu?
A: Buat multiple nodes + 1 gateway. ChirpStack support unlimited devices

### Q: Apa durasi battery ESP32?
A: ~300-400 hari dengan power bank 4200mAh (lihat analisis power)

### Q: Bisakah langsung deploy tanpa testing?
A: Tidak disarankan! Minimal test 24 jam di lab dulu

## Support & Contact

Untuk pertanyaan teknis:

1. **GitHub Issues** - Bug reports, features requests
2. **Discussion Forum** (jika ada) - General questions
3. **Email Authors** - nagatapct@gmail.com, tharizarevania18@gmail.com

Untuk documentation feedback:
- Buat PR dengan perbaikan
- Atau buat Issue "Documentation" tag

## Versioning & Updates

Project ini mengikuti Semantic Versioning (MAJOR.MINOR.PATCH):

- **v1.0.0** (current) - Initial release, stable dan tested
- **v1.1.x** - Bug fixes & improvements
- **v2.0.0** (planned) - Major features (e.g., multiple gateways, cloud integration)

Subscribe ke GitHub releases untuk update notifikasi.

---

**Last Updated:** September 2026
**Documentation Status:** Complete & Verified
**Ready for Publication:** Yes ✓

