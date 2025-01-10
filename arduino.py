import serial
import time

def send_to_arduino(result):
    try:
        ser = serial.Serial('COM3', 9600, timeout=1) 
        time.sleep(2)  
        ser.write(f'{result}\n'.encode('utf-8'))
        print(f"{result}")
    except Exception as e:
        print(f"{e}")
    finally:
        ser.close()

if __name__ == "__main__":
    print("Ok")
