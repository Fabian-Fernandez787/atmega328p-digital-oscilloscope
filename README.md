# Bare-Metal ATmega328P Digital Oscilloscope

A high-performance, bare-metal digital oscilloscope built on the ATmega328P microcontroller (Arduino Uno hardware) using raw AVR C and register manipulation (no Arduino framework/libraries).

## System Architecture & Features
- **Processor:** ATmega328P @ 16 MHz
- **Toolchain:** `avr-gcc`, `make`, `avrdude`
- **UART Driver:** Custom register-level USART driver running at 115200 Baud (Double Speed Mode `U2X0 = 1`, 8N1 frame format)

## Baud Rate & Register Setup
- **Target Baud Rate:** 115,200
- **U2X0 Prescaler:** Double Speed enabled (`UCSR0A |= (1 << U2X0)`)
- **UBRR0 Calculation:** 
  $$\text{UBRR0} = \frac{16,000,000}{8 \times 115,200} - 1 = 16.36 \approx 16$$
- **Baud Error:** $+2.1\%$

## How to Build & Flash
1. Connect Arduino Uno to host PC (Update `PORT` in `Makefile` if necessary).
2. Compile binary:
   ```bash
   make