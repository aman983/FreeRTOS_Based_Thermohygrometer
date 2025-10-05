# 🌡️ FreeRTOS-Based Thermohygrometer

A real-time temperature and humidity monitoring system utilizing **FreeRTOS** on an **AVR microcontroller** (e.g., ATmega328P). This project features bare-metal implementations of drivers for the **DHT22 sensor** and the **TM1638 display module**, demonstrating robust task scheduling and inter-task communication via FreeRTOS queues.

![Project Demo](assets/output.gif)

---

## 🚀 Key Features

* **Low-Level DHT22 Driver:** Custom, timing-critical implementation of the DHT22 single-wire protocol using direct AVR GPIO manipulation.
* **Bit-Banged TM1638 Driver:** A software-only serial driver for the TM1638, managing 7-segment displays, LEDs, and button input.
* **Decoupled Architecture:** Utilizes FreeRTOS queues to ensure strict separation between sensor logic, UI logic, and display output.
* **Real-Time Operation:** Demonstrates reliable, concurrent execution of periodic sampling and display updates.

---

## ⚙️ System Architecture

The system is structured around four independent FreeRTOS tasks communicating via queues.

### Task Map

| Task Name | Priority | Role | Update Frequency |
| :--- | :--- | :--- | :--- |
| **DHT\_Task** | `tskIDLE_PRIORITY+1` | Data Producer (Sensor Sampling) | Every $2$ seconds |
| **Display\_task** | $6$ | UI Hub (Display/LED Control & Button Polling) | Every $100 \text{ ms}$ |
| **Button\_Task** | `tskIDLE_PRIORITY+1` | UI Logic (Processes User Input) | Every $100 \text{ ms}$ |
| **vLEDFlashTask** | `tskIDLE_PRIORITY` | System Heartbeat (Liveness Check) | Every $500 \text{ ms}$ |

### 🔄 Queue Communication

The design ensures non-blocking data exchange, centralizing all UI interaction through the `Display_task`.

| Queue Name | Direction (Producer $\to$ Consumer) | Data Type | Purpose |
| :--- | :--- | :--- | :--- |
| `Display_Group1_Queue` | $\text{DHT\_Task} \to \text{Display\_task}$ | `float` | Temperature $(\text{}^{\circ}\text{C)}$ to be displayed on Segment Group 1. |
| `Display_Group2_Queue` | $\text{DHT\_Task} \to \text{Display\_task}$ | `float` | Humidity $(\%)$ to be displayed on Segment Group 2. |
| `Display_Led_Queue` | $\text{Button\_Task} \to \text{Display\_task}$ | `uint8_t` | Status counter/pattern to control the $\text{TM1638}$ **LEDs**. |
| `Display_Button_Queue` | $\text{Display\_task} \rightleftarrows \text{Button\_Task}$ | `uint16_t` | Raw $\text{TM1638}$ **button state bitmap** (uses **`xQueueOverwrite`**). |

---

## 🧠 Low-Level Driver Implementation

### 1. DHT22 Sensor Protocol (`get_dht` function)

The $\text{DHT22}$ requires precise microsecond timing, which is handled directly using the AVR's `_delay_us()` function and direct GPIO register manipulation ($\text{DDRD}$, $\text{PORTD}$, $\text{PIND}$).

* **Host Start Signal:** $18 \text{ ms LOW}$ pulse followed by $40 \text{ µs HIGH}$ pulse.
* **Data Read:** The code explicitly waits for the sensor's response handshake and then reads the $40$ bits of data.
* **Bit Decoding:** Data bits are decoded by checking the line state $\sim 35 \text{ µs}$ into the $\text{HIGH}$ pulse duration. A short pulse is a '$\text{0}$', and a long pulse is a '$\text{1}$'.
* **Error Handling:** Timeouts are implemented using a software counter (`timeout`) to prevent task blocking if the sensor fails to respond. The $\text{DHT\_Task}$ also uses $\text{UART}$ for logging read failures.

### 2. TM1638 Display Driver (`Display_task`)

The $\text{Display\_task}$ utilizes the bit-banged $\text{TM1638.h}$ driver to manage all aspects of the UI module via a 3-wire serial interface ($\text{STB}$, $\text{CLK}$, $\text{DIO}$).

* **Input Handling:** $\text{TM\_Button\_Read()}$ is polled every $100 \text{ ms}$. The result is immediately sent using `xQueueOverwrite()` to $\text{Display\_Button\_Queue}$, ensuring the $\text{Button\_Task}$ always receives the latest button state without accumulating stale events.
* **Output Handling:** It acts as a consumer for all display/LED queues. It blocks for only $10$ ticks on each queue, ensuring it remains responsive even if a producer (like $\text{DHT\_Task}$) is delayed.

| Segment Group | Data Source | Update Function |
| :--- | :--- | :--- |
| Group 1 (Left) | Temp $(\text{data}[1])$ from $\text{Display\_Group1\_Queue}$ | $\text{TM\_Display\_Float}(\text{temp}, 0)$ |
| Group 2 (Right) | Hum $(\text{data}[0])$ from $\text{Display\_Group2\_Queue}$ | $\text{TM\_Display\_Float}(\text{hum}, 1)$ |
| LEDs | Counter from $\text{Display\_Led\_Queue}$ | $\text{TM\_Display\_led}(\text{counter})$ |

---

## 🔩 Hardware Connections

| Peripheral | MCU Pin | Description |
| :--- | :--- | :--- |
| **DHT22 Data** | $\text{PD5}$ | Data line (requires $10 \text{k}$ pull-up) |
| **TM1638 STB** | $\text{PB0}$ | Strobe/Latch |
| **TM1638 CLK** | $\text{PB1}$ | Clock |
| **TM1638 DIO** | $\text{PB2}$ | Data I/O |
| **Status LED** | $\text{PB5}$ | System Heartbeat LED |
| **UART** | $\text{PD0/PD1}$ | Debug output ($9600$ baud) |

---

## 🔧 Software Setup & Building

### Prerequisites

* **AVR-GCC Toolchain**
* **FreeRTOS source** configured for AVR
* Project headers ($\text{UART.h}$, $\text{TM1638.h}$, $\text{Display.h}$)
* `make` and `avrdude`

### Execution Flow

1.  **Initialization (`main`):** $\text{UART}$ and $\text{Display}$ initialization routines are called. $\text{Display\_init}$ is crucial as it creates all necessary FreeRTOS queues and the $\text{Display\_task}$.
2.  **Task Creation:** All four tasks ($\text{vLEDFlashTask}$, $\text{Button\_Task}$, $\text{DHT\_Task}$, $\text{Display\_task}$) are created with their respective stack sizes and priorities.
3.  **Scheduler Start:** `vTaskStartScheduler()` begins cooperative multitasking.

### Compilation and Flashing

(Assuming a standard Makefile setup)

```bash
# Build the project
make all

# Flash to the microcontroller (example for ATmega328P via usbasp)
avrdude -p m328p -c usbasp -U flash:w:FreeRTOS_Based_Thermohygrometer.hex
