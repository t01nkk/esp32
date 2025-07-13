| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

Before project configuration and build, be sure to set the correct chip target using `idf.py set-target <chip_name>`.

### Hardware Required

-   A development board with normal LED or addressable LED on-board (e.g., ESP32-S3-DevKitC, ESP32-C6-DevKitC etc.)
-   A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

### Configure the Project

Open the project configuration menu (`idf.py menuconfig`).

In the `Example Configuration` menu:

-   Select the LED type in the `Blink LED type` option.
    -   Use `GPIO` for regular LED
    -   Use `LED strip` for addressable LED
-   If the LED type is `LED strip`, select the backend peripheral
    -   `RMT` is only available for ESP targets with RMT peripheral supported
    -   `SPI` is available for all ESP targets
-   Set the GPIO number used for the signal in the `Blink GPIO number` option.
-   Set the blinking period in the `Blink period in ms` option.

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.
