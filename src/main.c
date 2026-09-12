#include <avr/io.h>
#include <util/delay.h>

void uart_init(void) {
    UCSR0A |= (1 << U2X0);                  // Double speed mode
    UCSR0B |= (1 << TXEN0);                 // Enable TX transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1 frame format
    UBRR0H = 0;
    UBRR0L = 16;                            // 115200 Baud
}

void uart_transmit_byte(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));       // Wait until transmit buffer is ready
    UDR0 = data;
}

void adc_init(void) {
    ADMUX = (1 << REFS0);                   // AVCC reference, Pin A0
    DIDR0 |= (1 << ADC0D);                  // Disable digital buffer on A0
    // Enable ADC, Prescaler 128 (16MHz / 128 = 125kHz ADC clock)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void timer1_init_1khz_CTC(void) {
    TCCR1A = 0; 
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC Mode, Prescaler 64
    OCR1A = 249;                                        // Exact 1 ms period (1 kHz)
}

int main(void) {
    uart_init();
    adc_init();
    timer1_init_1khz_CTC();

    while (1) {
        // 1. Wait for Timer1 Compare Match flag to ensure precise 1 kHz timing
        while (!(TIFR1 & (1 << OCF1A)));
        TIFR1 |= (1 << OCF1A); // Clear flag by writing 1

        // 2. Start single ADC conversion
        ADCSRA |= (1 << ADSC);

        // 3. Wait for conversion to finish (~13 ADC clock cycles)
        while (ADCSRA & (1 << ADSC));

        // 4. Read 10-bit result
        uint16_t sample = ADC;

        // 5. Apply Phase 4 Framing Protocol
        uint8_t b1 = 0x80 | ((sample >> 7) & 0x07); // Sync header (Bit 7 = 1)
        uint8_t b2 = sample & 0x7F;                 // Payload data (Bit 7 = 0)

        // 6. Stream binary bytes over UART
        uart_transmit_byte(b1);
        uart_transmit_byte(b2);
    }

    return 0;
}