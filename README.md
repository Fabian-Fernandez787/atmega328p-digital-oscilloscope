# Bare-Metal ATmega328P Digital Oscilloscope

A high-performance, bare-metal digital oscilloscope built on the ATmega328P microcontroller (Arduino Uno hardware) using raw AVR C and register manipulation.

---

## 1. System Specifications
- **Microcontroller:** ATmega328P @ 16 MHz
- **Toolchain:** `avr-gcc`, `avr-libc`, `make`, `avrdude`
- **Architecture:** Bare-metal AVR C (No Arduino IDE / libraries)

---

## 2. Peripheral Modules

### UART Driver (Phase 1)
- **Baud Rate:** 115,200 (Double Speed Mode `U2X0 = 1`)
- **Frame Format:** 8N1 (8 data bits, No parity, 1 stop bit)
- **UBRR0 Calculation:** 
  $$\text{UBRR0} = \frac{16,000,000}{8 \times 115,200} - 1 = 16.36 \approx 16$$
- **Baud Rate Error:** $+2.1\%$

### ADC Sampling Engine (Phase 2)
- **Resolution:** 10-bit (0–1023 digital output)
- **Voltage Reference:** $V_{\text{CC}}$ (~4.9V USB bus)
- **Prescaler:** 128 ($\text{ADC Clock} = 16\text{ MHz} / 128 = 125\text{ kHz}$)
- **Input Channel:** ADC0 (Arduino Pin A0)
- **Optimization:** Disables digital input buffer on Pin A0 via `DIDR0 |= (1 << ADC0D)` to minimize signal noise.

---

## 3. How to Build & Flash

1. Connect the Arduino Uno to your PC and verify the serial port in `Makefile` (`PORT = COM5`).
2. Compile the source C code:
   ```bash
   make