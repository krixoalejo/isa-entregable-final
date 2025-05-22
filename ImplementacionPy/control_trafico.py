# control_trafico.py
import serial
import time
from collections import deque

ser = serial.Serial('/dev/cu.usbserial-0001', 115200, timeout=1)
HISTORY_SIZE = 5
traffic_history = deque(maxlen=HISTORY_SIZE)

def calculate_timing(v1, v2):
    diff = v1 - v2
    base = 15000  # 15s base
    
    if abs(diff) >= 3:
        return base + diff * 3000
    elif abs(diff) >= 1:
        return base + diff * 1500
    else:
        return base

try:
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='replace').strip()
            if line.startswith("TRAFFIC:"):
                data = line.split(":")[1]
                v1, v2 = map(int, data.split(","))
                
                new_time = calculate_timing(v1, v2)
                traffic_history.append(new_time)
                
                # Suavizado con media móvil
                avg_time = sum(traffic_history) / len(traffic_history)
                adjusted_time = int(avg_time * 0.7 + new_time * 0.3)
                
                ser.write(f"SET:{adjusted_time}\n".encode())
                print(f"Ajuste: {adjusted_time}ms | V1: {v1} | V2: {v2}")

except KeyboardInterrupt:
    print("\nInterrupción por usuario")
finally:
    ser.close()
