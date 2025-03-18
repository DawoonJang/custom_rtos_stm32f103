import serial
import time

def send_wmv_file(file_path, port="/dev/tty.usbserial-0001", baudrate=115200):
    try:
        ser = serial.Serial(port, baudrate, timeout=3)
        time.sleep(2)

        with open(file_path, 'rb') as file:
            file_data = file.read(1024)
            print(file_data)

            while file_data:
                ser.write(file_data)
                time.sleep(0.01) 

                file_data = file.read(1024)  # 계속해서 1024바이트씩 읽기

        print(f"File {file_path} successfully sent.")
        
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()  # 시리얼 포트 닫기

def test_rtos_output():
    port = "/dev/tty.usbserial-0001"
    baudrate = 115200

    try:
        ser = serial.Serial(port, baudrate, timeout=3)
        time.sleep(2)

        ser.flush() 
        output = ser.readline().decode().strip()

        print(f"Received: {output}") 
        assert "RTOS task finished" in output, "Expected output not found"

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == '__main__':
    send_wmv_file("music/1_ROV.wmv")

    # # RTOS 결과 확인
    # test_rtos_output()