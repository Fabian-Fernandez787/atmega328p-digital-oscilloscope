#include <avr/io.h>
#include <util/delay.h>

void uart_init(void) {
    UCSR0A |= (1 << U2X0); // Set Double Speed Mode
    UCSR0B |= (1 << TXEN0); // Enable Transmitter (TXEN0)
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); // Set Frame Format: 8bit, 1 stop bit, no parity (8n1)

    // Set Baud Rate to 115200
    UBRR0H = 0;
    UBRR0L = 16;
}

void uart_transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0))) {} // Busy-wait for transmit buffer to be empty

    UDR0 = data; // Put data into buffer, send byte automatically
}

void uart_print(const char* str){
    // while str == non-zero, send it over serial to transmit
    while (*str) {
        uart_transmit(*str++);
    }
}

int main (void) {
    uart_init();

    while (1) {
        uart_print("Oscilloscope Booting Up... \r\n");
        _delay_ms(1000);
    }

    return 0;
}