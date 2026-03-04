# AGENTS.md — ptouch-ng

## Project Overview

PTouch-NG is a FLOSS replacement firmware for the BigTreeTech Panda Touch, a touchscreen
companion device for Bambu Lab 3D printers. It targets the **ESP32-S3** (with octal PSRAM,
16 MB flash) and is built on **ESP-IDF 6.0** (C, not C++).

## Build System

ESP-IDF CMake-based build. All commands go through `idf.py`.

### Prerequisites

- ESP-IDF v6.0+ installed and sourced (`$IDF_PATH` set, `idf.py` on PATH).
- On Windows the ESP-IDF PowerShell/CMD environment must be activated first
  (`%IDF_PATH%\export.bat` or the ESP-IDF terminal shortcut).

### Commands

| Action | Command |
|---|---|
| **Full build** | `idf.py build` |
| **Clean build** | `idf.py fullclean && idf.py build` |
| **Flash** | `idf.py flash` |
| **Serial monitor** | `idf.py monitor` |
| **Flash + monitor** | `idf.py flash monitor` |
| **Menuconfig** | `idf.py menuconfig` |
| **Reconfigure CMake** | `idf.py reconfigure` |
| **Set target** | `idf.py set-target esp32s3` |

### Flashing — Serial Port Safety

**Before running any command that writes to the physical device** (`idf.py flash`,
`idf.py flash monitor`, `idf.py erase-flash`, etc.), you **MUST** confirm the serial port
with the user if there is any ambiguity. Concretely:

1. List all currently connected serial ports (e.g. `python -m serial.tools.list_ports -v`
   on all platforms, or `ls /dev/serial/by-id/` on Linux,
   `Get-CimInstance Win32_SerialPort` / `mode` on Windows).
2. Present the list to the user and **ask which port to use**.
3. Only then run the flash command with the explicit `-p <PORT>` flag.

Never guess the port. A wrong port can brick unrelated attached hardware.

### Component Management

Dependencies are declared in `main/idf_component.yml` and resolved by the ESP-IDF
component manager. Resolved versions are pinned in `dependencies.lock`.

| Dependency | Purpose |
|---|---|
| `fmauneko/pandatouch` | Board Support Package (display, touch, USB) |
| `lvgl/lvgl` (v9.5) | UI framework (transitive via pandatouch BSP) |
| `espressif/esp_lvgl_port` | LVGL integration with ESP-IDF |
| `espressif/cjson` | JSON parsing |
| `espressif/mqtt` | MQTT client |
| `joltwallet/littlefs` | LittleFS filesystem |
| `espressif/esp_new_jpeg` | JPEG decoder |

To add a dependency: edit `main/idf_component.yml`, then `idf.py build` (auto-fetches).
Downloaded components land in `managed_components/` (gitignored).

### Tests

No unit test framework is configured. Validation is done by building and flashing:
```
idf.py build          # Compile-time verification
idf.py flash monitor  # Runtime verification via serial output
```

### Partition Layout

Custom partition table (`partitions.csv`): dual OTA slots + LittleFS data partition.

## Target Hardware

- **SoC**: ESP32-S3 @ 240 MHz, octal SPI-RAM, 16 MB flash (QIO)
- **Display**: RGB LCD via `esp_lcd` + GT911 capacitive touch
- **Storage**: LittleFS partition, USB Mass Storage host
- **Connectivity**: WiFi STA, MQTT over TLS

## Code Style

### Language

**C11** (not C++). All source files use `.c` / `.h` extensions.

### Formatting

No `.clang-format` is present. Follow the existing style observed in `main/main.c`:

- **Indentation**: 4 spaces (no tabs)
- **Braces — functions**: opening brace on **its own line**
  ```c
  void app_main(void)
  {
      // ...
  }
  ```
- **Braces — control flow**: opening brace on **same line**
  ```c
  if (condition) {
      // ...
  } else if (other) {
      // ...
  }
  ```
- **Line width**: keep under ~100 columns; break long arg lists with alignment
- **Trailing commas**: use in designated initializers and enum lists
  ```c
  wifi_config_t cfg = {
      .sta = {
          .ssid = CREDENTIALS_WIFI_SSID,
          .password = CREDENTIALS_WIFI_PASSWORD,
      },
  };
  ```

### Naming Conventions

| Element | Convention | Example |
|---|---|---|
| Functions | `snake_case` | `wifi_event_handler` |
| Local variables | `snake_case` | `wifi_config`, `event_data` |
| Static globals | `snake_case` | `wifi_event_group` |
| Macros / constants | `UPPER_SNAKE_CASE` | `WIFI_CONNECTED_BIT` |
| Log tags | `static const char *TAG = "component-name";` | `"ptouch-ng"` |
| Credential defines | `CREDENTIALS_` prefix | `CREDENTIALS_WIFI_SSID` |

### Include Order

Group includes with a blank line between each group:

1. **Standard C library** (`<string.h>`, `<stdint.h>`, ...)
2. **FreeRTOS** (`"freertos/FreeRTOS.h"`, ...)
3. **ESP-IDF system** (`"esp_err.h"`, `"esp_log.h"`, `"nvs_flash.h"`, ...)
4. **Third-party / component** headers (`"cJSON.h"`, `"mqtt_client.h"`, ...)
5. **Project-local** headers (`"credentials.h"`, ...)

### Error Handling

- **ESP-IDF errors**: use `ESP_ERROR_CHECK()` for calls that must not fail.
  For recoverable errors, check `esp_err_t` explicitly:
  ```c
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ESP_ERROR_CHECK(nvs_flash_init());
  } else {
      ESP_ERROR_CHECK(ret);
  }
  ```
- **Never silently ignore errors**. Log a warning at minimum.

### Logging

Use ESP-IDF logging macros with the module `TAG`:
```c
static const char *TAG = "module-name";

ESP_LOGI(TAG, "Info message: %d", value);
ESP_LOGW(TAG, "Warning: %s", reason);
ESP_LOGE(TAG, "Error: %s", esp_err_to_name(err));
ESP_LOGD(TAG, "Debug details");
```

### Memory

- Prefer `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)` for large buffers (PSRAM).
- Use `heap_caps_get_free_size()` to monitor heap usage.
- LVGL is configured with 128 KB memory pool from PSRAM.

## Credentials & Secrets

`main/credentials.h` is **gitignored** and must never be committed. It defines:
```c
#define CREDENTIALS_WIFI_SSID       "..."
#define CREDENTIALS_WIFI_PASSWORD   "..."
#define CREDENTIALS_PRINTER_IP      "..."
#define CREDENTIALS_PRINTER_SN      "..."
#define CREDENTIALS_PRINTER_PASSCODE "..."
```
When creating new secrets, follow the `CREDENTIALS_` prefix convention and add to this file.

## LSP / Tooling

- **clangd** with ESP-IDF's `esp-clang` toolchain and `--clang-tidy` enabled.
- Compile commands are generated by CMake in `build/compile_commands.json` after first build.
- If clangd cannot find headers, run `idf.py reconfigure` to regenerate.

## Key Files

| Path | Purpose |
|---|---|
| `main/main.c` | Application entry point (`app_main`) |
| `main/credentials.h` | Local secrets (gitignored) |
| `main/certs/ca_cert.pem` | TLS CA certificate (embedded in binary) |
| `main/idf_component.yml` | Component dependencies |
| `main/CMakeLists.txt` | Source registration, embedded files |
| `CMakeLists.txt` | Top-level ESP-IDF project file |
| `sdkconfig.defaults` | Non-default SDK options (committed) |
| `sdkconfig` | Full generated config (gitignored) |
| `partitions.csv` | Custom flash partition table |
| `dependencies.lock` | Pinned dependency versions |
