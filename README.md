# Thermostat using STM32

This project implements a simple thermostat system using an STM32 microcontroller. The system reads temperature data from a DS18B20 sensor, displays the current temperature and setpoint on an LCD1602, and controls a cooling fan using an ON/OFF control algorithm.

The project was developed for the **Embedded System Programming** course and verified using **Proteus simulation**.

---

## 1. Project Overview

The thermostat monitors room temperature and controls a cooling fan based on a user-defined temperature setpoint.

Main behavior:

- Read temperature from a DS18B20 temperature sensor.
- Display current temperature, setpoint, operating mode, and fan status on LCD1602.
- Allow the user to adjust the setpoint using push buttons.
- Automatically turn the fan ON/OFF based on the temperature and setpoint.
- Provide Sleep, Normal, and Setpoint modes through a finite state machine.
- Verify the embedded software behavior in Proteus.

---

## 2. Main Features

- STM32-based embedded thermostat control.
- DS18B20 temperature measurement.
- LCD1602 display in 4-bit mode.
- Button-based user interface.
- ON/OFF fan control algorithm.
- Non-blocking temperature sampling design.
- Modular software architecture using Application, BSP, and Component Driver layers.
- Proteus simulation test cases for system verification.

---

## 3. Project Structure

```text
thermostat_final/
├── Core/
├── Drivers/
├── Source/
│   ├── App/
│   │   ├── app_main.c
│   │   └── app_main.h
│   ├── BSP/
│   │   ├── bsp_button.c
│   │   ├── bsp_button.h
│   │   ├── bsp_ds18b20.c
│   │   ├── bsp_ds18b20.h
│   │   ├── bsp_fan.c
│   │   ├── bsp_fan.h
│   │   ├── bsp_lcd.c
│   │   └── bsp_lcd.h
│   └── Components/
│       ├── delay/
│       ├── ds18b20/
│       └── lcd1602/
├── thermostat_final.ioc
├── STM32F401CEUX_FLASH.ld
├── STM32F401CEUX_RAM.ld
├── .project
├── .cproject
└── .mxproject
```

---

## 4. Software Architecture

The software is organized into multiple layers.

| Layer | Location | Responsibility |
|---|---|---|
| Application Layer | `Source/App` | Thermostat behavior, FSM, control logic, LCD content |
| BSP Layer | `Source/BSP` | Hardware abstraction for buttons, fan, LCD, and DS18B20 |
| Component Driver Layer | `Source/Components` | Low-level reusable drivers for DS18B20, LCD1602, and delay |
| MCU/System Driver | `Core/`, `Drivers/` | STM32 peripheral initialization and HAL/LL drivers |

The application layer communicates with hardware through BSP APIs instead of directly accessing GPIO pins or low-level driver details. This improves readability, maintainability, and portability.

---

## 5. Finite State Machine

The thermostat has three main states:

| State | Description | Fan Behavior |
|---|---|---|
| `S_THERMO_SLEEP` | Monitor-only mode. Temperature is displayed, but automatic cooling is disabled. | Fan forced OFF |
| `S_THERMO_NORMAL` | Normal thermostat mode. Temperature is compared with the setpoint. | Fan controlled automatically |
| `S_THERMO_SETPOINT` | Setpoint editing mode. User can change the setpoint using Up/Down buttons. | Fan control still active |

### State Transitions

| Current State | Button Event | Next State | Action |
|---|---|---|---|
| `S_THERMO_SLEEP` | Sleep | `S_THERMO_NORMAL` | Enable normal operation |
| `S_THERMO_NORMAL` | Sleep | `S_THERMO_SLEEP` | Turn fan OFF |
| `S_THERMO_NORMAL` | Set | `S_THERMO_SETPOINT` | Copy current setpoint to editable setpoint |
| `S_THERMO_SETPOINT` | Up | `S_THERMO_SETPOINT` | Increase editable setpoint if below maximum |
| `S_THERMO_SETPOINT` | Down | `S_THERMO_SETPOINT` | Decrease editable setpoint if above minimum |
| `S_THERMO_SETPOINT` | Set | `S_THERMO_NORMAL` | Save edited setpoint |
| Undefined case | Undefined | `S_THERMO_SLEEP` | Return to safe state and turn fan OFF |

<p align="center">
  <img src="fsm.png" alt="Thermostat FSM Diagram" width="800">
</p>

<p align="center">
  <b>Figure 1:</b> Thermostat finite state machine diagram
</p>

---

## 6. Build and Run

### Requirements

- STM32CubeIDE
- STM32CubeMX project file `.ioc`
- Proteus for simulation
- STM32F4 firmware package
- ARM GCC toolchain, usually included with STM32CubeIDE

### Steps

1. Clone this repository:

```bash
git clone https://github.com/NguyenTran-88/Thermostat-using-STM32.git
```

2. Open STM32CubeIDE.

3. Import the project:

```text
File > Import > Existing Projects into Workspace
```

4. Select the cloned project folder.

5. Build the project.

6. Load the generated firmware file into the STM32 model in Proteus.

7. Run the Proteus simulation.

---

## 7. Proteus Simulation Test Cases

| Test Case | Condition | Expected Result | Result |
|---|---|---|---|
| TC01 | System starts | LCD shows initial state | Pass |
| TC02 | Temperature changes | LCD updates current temperature | Pass |
| TC03 | Temperature > setpoint | Fan turns ON | Pass |
| TC04 | Temperature <= setpoint | Fan turns OFF | Pass |
| TC05 | Up button pressed | Setpoint increases by 1°C | Pass |
| TC06 | Down button pressed | Setpoint decreases by 1°C | Pass |
| TC07 | Set button pressed | System enters or exits Setpoint mode | Pass |
| TC08 | Sleep button pressed | System changes between Normal and Sleep mode | Pass |

<p align="center">
  <img src="simulate.png" alt="Proteus Simulation Model" width="800">
</p>

<p align="center">
  <b>Figure 2:</b> Proteus simulation model of the thermostat system
</p>
---


## Reference

- nimaltd, “Non-blocking DS18B20 Library for STM32,” GitHub.  
  Available: https://github.com/nimaltd/ds18b20
