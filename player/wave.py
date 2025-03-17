import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import tkinter as tk
import serial
import time
import struct
import threading
import serial.tools.list_ports
from tkinter import scrolledtext

class UartHandler:
    def __init__(self, log_callback):
        self.device = None
        self.log_callback = log_callback
        self.port = self.find_cp2102_port()
        if self.port:
            self.device = self.set_uart_comm(self.port)
            if self.device:
                self.start_receiving()
        else:
            self.log_callback("[ERROR] CP2102 포트를 찾을 수 없습니다.")

    def find_cp2102_port(self):
        ports = serial.tools.list_ports.comports()
        for port in ports:
            if port.vid == 0x10C4 and port.pid == 0xEA60:
                return port.device
        return None

    def set_uart_comm(self, port, baudrate=115200):
        try:
            ser = serial.Serial(port, baudrate, parity='N', stopbits=1, bytesize=8, timeout=3)
            self.log_callback(f"[INFO] Connected to {port} at {baudrate} baud")
            return ser
        except serial.SerialException as e:
            self.log_callback(f"[ERROR] Serial error: {e}")
        except Exception as e:
            self.log_callback(f"[ERROR] Unexpected error: {e}")
        return None

    def start_receiving(self):
        thread = threading.Thread(target=self.uart_receive, daemon=True)
        thread.start()

    def uart_receive(self):
        while True:
            if self.device:
                self.device.flush()
                output = self.device.readline().decode().strip()
                if output:
                    self.log_callback(f"[RX] {output}")
            time.sleep(0.01)

    def send_wave_data(self, frequency, sampling_rate, duration):
        if not self.device:
            self.log_callback("[ERROR] Device not connected!")
            return
        
        packet = bytearray(10)
        packet[:2] = struct.pack("<H", 32555)
        packet[2:4] = struct.pack("<H", frequency)
        packet[4:6] = struct.pack("<H", sampling_rate)
        packet[6:8] = struct.pack("<H", duration)
        packet[8:10] = struct.pack("<H", 32666)
        
        self.log_callback(f"[TX] Sending: f={frequency}, sr={sampling_rate}, d={duration}")
        for byte in packet:
            self.device.write(byte.to_bytes())
            time.sleep(0.05)

class SineWaveGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Sine Wave Animation with UART Log")
        self.setup_ui()
        self.uart = UartHandler(self.log_message)
        self.ani = None

    def setup_ui(self):
        main_frame = tk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True)

        # Matplotlib Figure 생성
        self.fig, self.ax = plt.subplots()
        self.canvas = FigureCanvasTkAgg(self.fig, master=main_frame)
        self.canvas.get_tk_widget().pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # UART 로그 창
        log_frame = tk.Frame(main_frame)
        log_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)
        self.log_text = scrolledtext.ScrolledText(log_frame, height=20, width=40, state=tk.DISABLED)
        self.log_text.pack(fill=tk.BOTH, expand=True)

        # 입력 필드 & 버튼 UI
        control_frame = tk.Frame(self.root)
        control_frame.pack()

        tk.Label(control_frame, text="Frequency (Hz):").grid(row=0, column=0)
        self.freq_entry = tk.Entry(control_frame)
        self.freq_entry.grid(row=0, column=1)
        self.freq_entry.insert(0, "5")

        tk.Label(control_frame, text="Sampling Rate (Hz):").grid(row=1, column=0)
        self.sampling_entry = tk.Entry(control_frame)
        self.sampling_entry.grid(row=1, column=1)
        self.sampling_entry.insert(0, "100")

        tk.Label(control_frame, text="Duration (s):").grid(row=2, column=0)
        self.duration_entry = tk.Entry(control_frame)
        self.duration_entry.grid(row=2, column=1)
        self.duration_entry.insert(0, "2")

        self.plot_button = tk.Button(control_frame, text="Plot", command=self.plot_animation)
        self.plot_button.grid(row=3, columnspan=2)

        # 그래프 초기화
        self.ax.set_xlim(0, 2)
        self.ax.set_ylim(-1.2, 1.2)
        self.ax.set_xlabel("Time [s]")
        self.ax.set_ylabel("Amplitude")
        self.ax.set_title("Sine Wave Animation")
        self.line, = self.ax.plot([], [], lw=2)

    def log_message(self, message):
        self.log_text.config(state=tk.NORMAL)
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.yview(tk.END)
        self.log_text.config(state=tk.DISABLED)

    def plot_animation(self):
        try:
            frequency = int(self.freq_entry.get())
            sampling_rate = int(self.sampling_entry.get())
            duration = int(self.duration_entry.get())
        except ValueError:
            self.log_message("[ERROR] 올바른 숫자를 입력하세요.")
            return

        self.uart.send_wave_data(frequency, sampling_rate, duration)
        t = np.linspace(0, duration, int(sampling_rate * duration), endpoint=False)
        y_data = np.sin(2 * np.pi * frequency * t)
        self.ax.set_xlim(0, duration)
        self.ax.set_ylim(-1.2, 1.2)
        self.ax.set_title(f"Sine Wave (f={frequency}Hz, sr={sampling_rate}Hz, d={duration}s)")
        self.line.set_data([], [])
        
        def update(frame):
            self.line.set_data(t[:frame], y_data[:frame])
            return self.line,

        if self.ani:
            self.ani.event_source.stop()
        self.ani = animation.FuncAnimation(self.fig, update, frames=len(t), interval=20, blit=False)
        self.canvas.draw()

if __name__ == "__main__":
    root = tk.Tk()
    app = SineWaveGUI(root)
    root.mainloop()
