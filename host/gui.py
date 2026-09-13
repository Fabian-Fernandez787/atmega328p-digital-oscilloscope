import sys
import time
import collections
import threading
import serial
import numpy as np
from PyQt6 import QtWidgets, QtCore
import pyqtgraph as pg

# --- Configuration ---
PORT = 'COM5'
BAUD = 115200
BUFFER_SIZE = 1000  # 1 second of data at 1 kHz sample rate


class SerialReaderThread(QtCore.QThread):
    """Background thread to handle high-speed serial packet parsing."""
    sample_received = QtCore.pyqtSignal(float)

    def __init__(self, port, baud):
        super().__init__()
        self.port = port
        self.baud = baud
        self.running = True

    def run(self):
        try:
            ser = serial.Serial(self.port, self.baud, timeout=1)
            time.sleep(2)  # Allow Arduino DTR reset
            ser.reset_input_buffer()
        except Exception as e:
            print(f"Error opening serial port {self.port}: {e}")
            self.running = False
            return

        while self.running:
            # Look for Sync Header (Bit 7 == 1)
            raw_b1 = ser.read(1)
            if not raw_b1:
                continue

            b1_val = raw_b1[0]

            if b1_val & 0x80:
                raw_b2 = ser.read(1)
                if not raw_b2:
                    continue

                b2_val = raw_b2[0]

                if not (b2_val & 0x80):
                    # Reconstruct 10-bit ADC sample
                    adc_val = ((b1_val & 0x07) << 7) | (b2_val & 0x7F)
                    voltage = (adc_val * 5.0) / 1023.0
                    self.sample_received.emit(voltage)

        ser.close()

    def stop(self):
        self.running = False
        self.wait()


class OscilloscopeGUI(QtWidgets.QMainWindow):
    """Main Real-Time Oscilloscope GUI."""
    def __init__(self):
        super().__init__()
        self.setWindowTitle("ATmega328P Bare-Metal Digital Oscilloscope")
        self.resize(1000, 600)

        # Ring buffer for sliding window data storage
        self.data_buffer = collections.deque(maxlen=BUFFER_SIZE)
        self.data_buffer.extend([0.0] * BUFFER_SIZE)
        
        # Time axis vector (0 to 1000 ms)
        self.time_axis = np.linspace(0, BUFFER_SIZE, BUFFER_SIZE)

        # Set up GUI Layout
        central_widget = QtWidgets.QWidget()
        self.setCentralWidget(central_widget)
        layout = QtWidgets.QVBoxLayout(central_widget)

        # Configure PyQtGraph Plot Widget
        self.plot_widget = pg.PlotWidget()
        self.plot_widget.setBackground('#121212')  # Dark theme background
        self.plot_widget.showGrid(x=True, y=True, alpha=0.3)
        self.plot_widget.setYRange(-0.2, 5.2)       # 0 - 5V range with margin
        self.plot_widget.setLabel('left', 'Voltage', units='V')
        self.plot_widget.setLabel('bottom', 'Time', units='ms')
        
        # Plot Curve Styling (Cyan phosphor glow)
        pen = pg.mkPen(color='#00FFCC', width=2)
        self.curve = self.plot_widget.plot(self.time_axis, list(self.data_buffer), pen=pen)
        layout.addWidget(self.plot_widget)

        # Start Background Serial Thread
        self.serial_thread = SerialReaderThread(PORT, BAUD)
        self.serial_thread.sample_received.connect(self.update_data)
        self.serial_thread.start()

        # GUI Refresh Timer (~60 FPS update rate)
        self.timer = QtCore.QTimer()
        self.timer.setInterval(16)  # ~16 ms = 60 Hz
        self.timer.timeout.connect(self.update_plot)
        self.timer.start()

    def update_data(self, voltage):
        """Append incoming sample from serial thread to ring buffer."""
        self.data_buffer.append(voltage)

    def update_plot(self):
        """Redraw curve on UI thread."""
        self.curve.setData(self.time_axis, list(self.data_buffer))

    def closeEvent(self, event):
        """Clean shutdown of thread on app exit."""
        self.serial_thread.stop()
        event.accept()


def main():
    app = QtWidgets.QApplication(sys.argv)
    gui = OscilloscopeGUI()
    gui.show()
    sys.exit(app.exec())


if __name__ == '__main__':
    main()