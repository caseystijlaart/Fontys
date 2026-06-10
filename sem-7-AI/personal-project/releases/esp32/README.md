# ESP32 firmware releases

App images built from `src/firmware` (`esp32_provisioning` environment, `huge_app.csv` partition table). No WiFi credentials or API keys are embedded — devices are provisioned over BLE by the app.

## Flashing

Easiest (builds and flashes bootloader + partitions + app in one step):

```sh
cd src/firmware
platformio run -e esp32_provisioning -t upload
```

To flash a release `.bin` directly onto a device that already has a bootloader and the `huge_app` partition table:

```sh
esptool.py --chip esp32 --baud 460800 write_flash 0x10000 firmware-v<version>.bin
```

A factory-fresh chip additionally needs `bootloader.bin` at `0x1000` and `partitions.bin` at `0x8000` (both produced by the PlatformIO build in `.pio/build/esp32_provisioning/`).

## Versions

| Version | Notes |
|---|---|
| v1.1.0 | BLE provisioning, TinyML (TFLM) inference, Supabase sync, trigger_measurement / trigger_reset command polling |
| v1.0.0 | Initial release |
