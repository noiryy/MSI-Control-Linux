# MSI Control GUI (Qt6)

A minimal, modern Qt6 desktop app for switching MSI embedded controller shift modes, assigning keybinds, and
toggling cooler boost.

## Features

- Switch between **eco**, **comfort**, and **turbo** scenarios.
- Assign custom keybinds for each scenario.
- Toggle cooler boost with a single button.
- Dark/light gradient themes and rounded buttons.

## Dependencies

 - [msi-ec](https://github.com/BeardOverflow/msi-ec)

## How to access the MSI embedded controller

The MSI embedded controller is exposed through sysfs. The app uses the same paths your driver exposes:

- **Available shift modes**
  ```bash
  sudo cat /sys/devices/platform/msi-ec/available_shift_modes
  ```
- **Change shift mode**
  ```bash
  echo turbo | sudo tee /sys/devices/platform/msi-ec/shift_mode
  ```
- **Toggle cooler boost**
  ```bash
  echo on | sudo tee /sys/devices/platform/msi-ec/cooler_boost
  ```

Each action requires root; the app uses `pkexec` to request authentication.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/msi-ctl-gui
```

## Install

Use the installer script:

```bash
./installer.sh
```
