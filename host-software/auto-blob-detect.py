# CEG4195 Project
# Author: Alex Gordon
# Last revision: 3-29-26

# ===== 1.0 DEPENDENCIES & MACROS ===== 
#!/usr/bin/env python3
import serial
import time
import requests
import sys

# ===== 2.0 CONFIG AND GLOBALS ===== 
SERIAL_PORT = '/dev/ttyUSB0' # !!!! VERIFY THIS ON KLIPPER HOST !!!!
BAUD_RATE = 115200

MOONRAKER_URL = "http://localhost:7125/printer/gcode/script" 
TIMEOUT_SECONDS = 10

# ===== 3.0 FUNCTIONS ===== 
def run_klipper_macro(macro_name):
    """inject G-Code macro into klipper's execution queue via moonraker"""
    try:
        payload = {"script": macro_name}
        response = requests.post(MOONRAKER_URL, params=payload)
        if response.status_code == 200:
            print(f"MOONRAKER: Successfully queued {macro_name}")
        else:
            print(f"MOONRAKER ERR: Failed to trigger {macro_name} - HTTP {response.status_code}")
    except Exception as e:
        print(f"CONNECTION ERR: {e}")

def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
        time.sleep(2)
        
        ser.reset_input_buffer() 
        
        ser.write(b"CHECK_NOZZLE\n")
        start_time = time.time()
        
        while (time.time() - start_time) < TIMEOUT_SECONDS:
            if ser.in_waiting > 0:
                response = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if not response:
                    continue
                
                if response == "CLEAN":
                    print("SAFE STATE: CLEAN. Resuming.")
                    run_klipper_macro("AI_STATE_CLEAN")
                    sys.exit(0)
                    
                elif response == "EXTRUDED":
                    print("STATE: EXTRUDED. Triggering Silicone Wipe Loop.")
                    run_klipper_macro("AI_STATE_EXTRUDED")
                    sys.exit(0)
                    
                elif response == "BLOB":
                    print("CRITICAL: BLOB DETECTED! Triggering Full Clean Loop.")
                    run_klipper_macro("AI_STATE_BLOB")
                    sys.exit(0)
                    
                elif response in ["CAM_FAIL", "CAP_FAIL", "ML_ERR"]:
                    print(f"HW ERR: {response}. Leaving printer paused.")
                    # we do not queue a resume macro, effectively halting Klipper safely.
                    sys.exit(2)
                    
        print("HW ERR: ESP32 TIMED OUT. Leaving printer paused.")
        sys.exit(3)
        
    except serial.SerialException as e:
        print(f"HW ERR: SERIAL PORT CONNECTION, {e}")
        sys.exit(4)

if __name__ == "__main__":
    main()