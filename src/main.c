#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

void uart_init(void) {
    UCSR0A |= (1 << U2X0); // Set Double Speed Mode
    UCSR0B |= (1 << TXEN0); // Enable Transmitter (TXEN0)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Set Frame Format: 8bit, 1 stop bit, no parity (8n1)

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

void adc_init(void) {
    ADMUX = (1 << REFS0);   // Select AVcc with external capacitor at AREF pin

    DIDR0 |= (1 << ADC0D);  // Disable digital input buffer on ADC0 (Pin A0) to reduce noise

    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC and set Prescaler to 128 (16 MHz / 128 = 125 kHz)
}

uint16_t adc_read(uint8_t channel) {
    channel &= 0x07;    // Ensure channel is between 0 and 7

    ADMUX = (ADMUX & 0xF0) | channel;   // Clear previous channel selection bits (bits MUX0-MUX3 in ADMUX)

    ADCSRA |= (1 << ADSC);  // Start single conversion by setting ADSC bit

    while (ADCSRA & (1 << ADSC));   // Wait for conversion to complete (ADSC drops back to 0)

    return ADC;  // Return 10-bit combined result from ADCL and ADCH
}

int main (void) {
    uart_init();
    adc_init();

    char buffer[32];

    while (1) {
        uint16_t adc_val = adc_read(0);     // Read raw 10bit value from ADC Channel 0 (Pin)
        uint32_t voltage_mv = ((uint32_t)adc_val * 5000) / 1024;    // Convert raw ADC value to (mV)

        sprintf(buffer, "ADC: %4u | Volts: %u.%03u V\r\n",  // Format data into a printable string
                adc_val,
                (unsigned int)(voltage_mv / 1000),
                (unsigned int)(voltage_mv % 1000));

        uart_print(buffer);
        
        _delay_ms(200); // Sample 5 times per second for terminal display

    }

    return 0;
}