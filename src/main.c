#include <avr/io.h>
#include <avr/interrupt.h>

void uart_init(void) {
    UCSR0A |= (1 << U2X0);                  // Double speed mode
    UCSR0B |= (1 << TXEN0);                 // Enable transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1 frame format
    UBRR0H = 0;
    UBRR0L = 16;                            // 115200 Baud
}

static inline void uart_transmit_byte(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));       // Wait until transmit buffer is ready
    UDR0 = data;
}

void adc_auto_trigger_init(void) {
    ADMUX = (1 << REFS0);                   // AVCC reference, Channel ADC0 (Pin A0)
    DIDR0 |= (1 << ADC0D);                  // Disable digital input buffer on A0
    
    // Enable ADC, Enable Auto-Trigger (ADATE), Enable ADC Interrupt (ADIE), Prescaler 128
    ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADIE) | 
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Select Timer1 Compare Match B as the Auto-Trigger Source (ADTS2:0 = 101)
    ADCSRB = (1 << ADTS2) | (1 << ADTS0);
}

void timer1_init_1khz(void) {
    TCCR1A = 0; 
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC Mode, Prescaler 64
    OCR1A = 249;                                        // Set period to 1ms (1 kHz)
    OCR1B = 249;                                        // Trigger ADC match event at top
}

// Hardware Interrupt: Fires automatically only when an ADC conversion completes
ISR(ADC_vect) {
    uint16_t sample = ADC;

    // Transmit raw 16-bit sample as 2 binary bytes (High byte, Low byte)
    // 2 bytes per sample @ 1 kHz = 2,000 bytes/sec (Well under 11,520 byte/sec limit!)
    uart_transmit_byte((uint8_t)(sample >> 8)); 
    uart_transmit_byte((uint8_t)(sample & 0xFF));
}

int main(void) {
    uart_init();
    adc_auto_trigger_init();
    timer1_init_1khz();

    // Enable Global Interrupts
    sei();

    while (1) {
        // Main loop is 100% idle! 
        // Conversions and data transmission are handled entirely by hardware interrupts.
    }

    return 0;
}