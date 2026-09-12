import serial
import time

PORT = 'COM5'
BAUD = 115200

def main():
    # Open serial port
    ser = serial.Serial(PORT, BAUD, timeout=1)
    
    # Wait for Arduino DTR reset to complete
    time.sleep(2)
    
    # Clear out any stale bytes left in the OS buffer
    ser.reset_input_buffer()
    
    print(f"Connected to {PORT} at {BAUD} baud. Streaming data...\n")

    try:
        while True:
            # Read 1 byte and check for Sync Header (Bit 7 == 1)
            raw_b1 = ser.read(1)
            if not raw_b1:
                continue

            b1_val = raw_b1[0]

            if b1_val & 0x80:  # Header byte detected
                raw_b2 = ser.read(1)
                if not raw_b2:
                    continue

                b2_val = raw_b2[0]

                if not (b2_val & 0x80):  # Payload byte validated
                    # Reconstruct 10-bit ADC integer
                    adc_val = ((b1_val & 0x07) << 7) | (b2_val & 0x7F)
                    voltage = (adc_val * 5.0) / 1023.0
                    
                    print(f"Raw ADC: {adc_val:4d} | Voltage: {voltage:.3f} V")
                else:
                    print("Framing Error: Expected Payload byte, got Header")

    except KeyboardInterrupt:
        print("\nStopping parser...")
        ser.close()

if __name__ == "__main__":
    main()