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
from tkinter import scrolledtext, ttk
import re
from decimal import Decimal


SIGNAL_FREQ = 2000
SAMPLE_RATE = 4000
PI = np.pi

class UartHandler:
    def __init__(self, log_callback, plot_callback):
        self.device = None
        self.log_callback = log_callback
        self.plot_callback = plot_callback
        self.port = self.find_cp2102_port()
        self.data_buffer = []
        self.recording = False
        self.thread = threading.Thread(target=self.uart_receive, daemon=True)

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
        self.thread.start()

    def uart_receive(self):
        cnt = 0
        while True:
            if self.device:
                recvData = self.device.readline().decode().strip()
                # print(recvData)
                if recvData:
                    if recvData == "S":
                        filterType = int(self.device.readline().decode().strip())
                        # print(filterType)
                        self.recording = True
                        self.data_buffer = []
                        cnt = 0
                        # print("[INFO] Start of transmission detected")
                    elif recvData == "E" and self.recording:
                        self.recording = False
                        self.plot_callback(self.data_buffer, filterType)
                        # print("[INFO] End of transmission detected")
                        if len(self.data_buffer) == 128:
                            self.log_callback(f"[INFO] Filtered Data Receive Complete,", filter = filterType)
                    else:
                        cnt += 1
                        try:
                            self.data_buffer.append(Decimal(recvData))
                        except:
                            self.log_callback(f"[ERROR] Invalid number received: {recvData}")

    def start_sending(self, selectedSignal, filterType):
        if self.device:
            data_byte = ((filterType & 0x0F) << 4) | (selectedSignal & 0x0F)
            self.device.write(int(data_byte).to_bytes())
            # self.log_callback(f"[TX] Sent: {bin(data_byte)}")
            time.sleep(0.1)
            
    def stop_receiving(self):
        if self.device:
            self.device.close()
            
        
class SineWaveGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Sine Wave Animation with UART Log")
        self.root.geometry("1600x800")  # 창 크기 확대
        self.filterType = 1  # Default to FIR
        self.setup_ui()
        self.uart = UartHandler(self.log_message, self.plot_received_data)
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.signal=0

    def setup_ui(self):
        main_frame = tk.Frame(self.root, padx=10, pady=10)
        main_frame.pack(fill=tk.BOTH, expand=True)

        self.fig, (self.ax, self.ax_received) = plt.subplots(2, 1, figsize=(8, 8), constrained_layout=True)  # 그래프 간 간격 조정
        self.canvas = FigureCanvasTkAgg(self.fig, master=main_frame)
        self.canvas.get_tk_widget().pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=10)

        control_frame = tk.Frame(main_frame, padx=5, pady=5)
        control_frame.pack(side=tk.RIGHT, fill=tk.Y)

        log_frame = tk.LabelFrame(control_frame, text="UART Log", padx=5, pady=5)
        log_frame.pack(fill=tk.BOTH, expand=True)
        self.log_text = scrolledtext.ScrolledText(log_frame, height=10, width=35, state=tk.DISABLED)
        self.log_text.pack(fill=tk.BOTH, expand=True)

        config_frame = tk.LabelFrame(control_frame, text="Controls", padx=5, pady=5)
        config_frame.pack(fill=tk.X, expand=False, pady=5)

        tk.Label(config_frame, text="Select Signal Preset:").grid(row=0, column=0, sticky='w', pady=2)
        self.Combo_Signal = ttk.Combobox(config_frame, values=[1, 2, 3], state="readonly", width=5)
        self.Combo_Signal.grid(row=0, column=1, pady=2)
        self.Combo_Signal.current(0)

        self.Radio_FilterType = tk.IntVar(value=1)
        tk.Label(config_frame, text="Select Filter Type:").grid(row=1, column=0, sticky='w', pady=2)
        
        self.fir_radio = tk.Radiobutton(config_frame, text="FIR", variable=self.Radio_FilterType, value=1)
        self.iir_radio = tk.Radiobutton(config_frame, text="IIR", variable=self.Radio_FilterType, value=2)
        self.fir_radio.grid(row=1, column=1)
        self.iir_radio.grid(row=1, column=2)

        self.plot_button = tk.Button(config_frame, text="Plot", command=self.plot_graph)
        self.plot_button.grid(row=2, columnspan=2, pady=5)
        
        self.ax.set_xlabel("Sample Index")
        self.ax.set_ylabel("Amplitude")
        self.ax.set_title("Generated Waveform")
        
        self.ax_received.set_xlabel("Sample Index")
        self.ax_received.set_ylabel("Amplitude")
        self.ax_received.set_title("Filtered Result")

    def log_message(self, message, filter = 0):
        self.log_text.config(state=tk.NORMAL)

        if filter == 0:
            self.log_text.insert(tk.END, message + "\n")
        else:
            if filter == 1:
                filterTypeStr = "LPF"
            elif filter == 2:
                filterTypeStr = "HPF"
            elif filter == 3:
                filterTypeStr = "BPF"
            self.log_text.insert(tk.END, message +f" Filter: {filterTypeStr}" "\n")
            
        self.log_text.yview(tk.END)
        self.log_text.config(state=tk.DISABLED)

    def plot_graph(self):
        self.filterType = int(self.Radio_FilterType.get())
        selectedSignal = int(self.Combo_Signal.get())
        self.ax_received.clear()
        self.uart.start_sending(selectedSignal, self.filterType)
        self.signal = self.generate_signal(selectedSignal)
        self.ax.clear()
        self.ax.plot(self.signal, lw=2, label="Generated Signal")
        
        self.ax.set_xlabel("Sample Index")
        self.ax.set_ylabel("Amplitude")
        self.ax.set_title("Generated Waveform")
        self.ax.legend()
        self.ax.grid(True)
        self.canvas.draw()
    
    def plot_received_data(self, data, filterType):
        if filterType == 1:
            filterTypeStr = "LPF"
        elif filterType == 2:
            filterTypeStr = "HPF"
        elif filterType == 3:
            filterTypeStr = "BPF"

        self.ax_received.clear()
        if filterType == 0:
            filterTypeStr = "No filter"
            # print("nofilter")
            self.ax_received.plot(self.signal)
        else:
            self.ax_received.plot(data)
        self.ax_received.set_xlabel("Sample Index")
        self.ax_received.set_ylabel("Amplitude")
        self.ax_received.set_title(f"Filtered Result: {filterTypeStr}")
        self.ax_received.grid(True)
        self.canvas.draw()
            
    def generate_signal(self, signalIdx, samples=512):
        i = np.arange(128)
        signals = [
            0,
            0.5 * np.sin(2 * PI * 1906 * i / SAMPLE_RATE) +
            0.75 * np.sin(2 * PI * 1200 * i / SAMPLE_RATE) +
            1.2 * np.cos(2 * PI * 800 * i / SAMPLE_RATE) +
            1.5 * np.sin(2 * PI * 500 * i / SAMPLE_RATE) +
            0.8 * np.cos(2 * PI * 300 * i / SAMPLE_RATE),
            
            np.sin(2 * PI * 2100 * i / SAMPLE_RATE) +
            0.9 * np.cos(2 * PI * 1400 * i / SAMPLE_RATE) +
            0.7 * np.sin(2 * PI * 900 * i / SAMPLE_RATE) +
            1.3 * np.cos(2 * PI * 600 * i / SAMPLE_RATE) +
            1.1 * np.sin(2 * PI * 400 * i / SAMPLE_RATE),
            
            np.cos(2 * PI * 1600 * i / SAMPLE_RATE) +
            1.2 * np.sin(2 * PI * 1300 * i / SAMPLE_RATE) +
            0.6 * np.cos(2 * PI * 700 * i / SAMPLE_RATE) +
            1.4 * np.sin(2 * PI * 500 * i / SAMPLE_RATE) +
            0.9 * np.cos(2 * PI * 200 * i / SAMPLE_RATE)
        ]
        return signals[signalIdx]
    
    def on_closing(self):
        self.uart.stop_receiving()
        self.root.quit()
        self.root.destroy()
        
if __name__ == "__main__":
    root = tk.Tk()
    app = SineWaveGUI(root)
    root.mainloop()