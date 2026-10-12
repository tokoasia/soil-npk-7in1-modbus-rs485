import minimalmodbus
import serial
import time
import struct

def read_7in1_sensor():
    try:
        # Configure the instrument - CHANGE COM PORT IF NEEDED!
        instrument = minimalmodbus.Instrument('COM8', 1)  # Change COM3 to your actual port
        instrument.serial.baudrate = 4800
        instrument.serial.bytesize = 8
        instrument.serial.parity = serial.PARITY_NONE
        instrument.serial.stopbits = 1
        instrument.serial.timeout = 1

        print("7-in-1 NPK Sensor Reader Starting...")
        print("Press Ctrl+C to stop\n")

        while True:
            try:
                # Read all parameters in one go (registers 0x0000 to 0x0006)
                data = instrument.read_registers(0, 7)  # Read 7 registers starting from address 0
                
                # Process the data
                moisture = data[0] / 10.0  # Register 40001 - Moisture (10x larger)
                temperature_raw = data[1]  # Register 40002 - Temperature (10x larger)
                conductivity = data[2]     # Register 40003 - Conductivity (us/cm)
                ph = data[3] / 10.0        # Register 40004 - PH (10x larger)
                nitrogen = data[4]         # Register 40005 - Nitrogen (mg/kg)
                phosphorus = data[5]       # Register 40006 - Phosphorus (mg/kg)
                potassium = data[6]        # Register 40007 - Potassium (mg/kg)
                
                # Handle negative temperature (two's complement)
                if temperature_raw > 32767:
                    temperature = (temperature_raw - 65536) / 10.0
                else:
                    temperature = temperature_raw / 10.0
                
                # Display results
                print("\n" + "="*60)
                print(f"Moisture:      {moisture:6.1f} %")
                print(f"Temperature:   {temperature:6.1f} °C")
                print(f"Conductivity:  {conductivity:6d} us/cm")
                print(f"PH:            {ph:6.1f}")
                print(f"Nitrogen:      {nitrogen:6d} mg/kg")
                print(f"Phosphorus:    {phosphorus:6d} mg/kg")
                print(f"Potassium:     {potassium:6d} mg/kg")
                print("="*60)
                
                time.sleep(5)  # Read every 5 seconds
                
            except Exception as e:
                print(f"\nError reading sensor: {e}")
                print("Check connections and try again...")
                time.sleep(5)

    except KeyboardInterrupt:
        print("\n\nStopped by user")
    except Exception as e:
        print(f"Failed to initialize: {e}")
        print("Please check:")
        print("1. COM port number")
        print("2. Sensor power (4.5-30V DC)")
        print("3. Wiring connections")
        print("4. Baud rate (default: 4800)")

def read_individual_parameters():
    """Alternative method: Read each parameter individually"""
    try:
        instrument = minimalmodbus.Instrument('COM3', 1)
        instrument.serial.baudrate = 4800
        instrument.serial.bytesize = 8
        instrument.serial.parity = serial.PARITY_NONE
        instrument.serial.stopbits = 1
        instrument.serial.timeout = 1

        print("Reading individual parameters...")
        
        while True:
            try:
                # Read moisture (register 0x0000 = 0)
                moisture = instrument.read_register(0, 1) / 10.0
                
                # Read temperature (register 0x0001 = 1)
                temp_raw = instrument.read_register(1, 0)
                if temp_raw > 32767:
                    temperature = (temp_raw - 65536) / 10.0
                else:
                    temperature = temp_raw / 10.0
                
                # Read conductivity (register 0x0002 = 2)
                conductivity = instrument.read_register(2, 0)
                
                # Read PH (register 0x0003 = 3)
                ph = instrument.read_register(3, 1) / 10.0
                
                # Read NPK values
                nitrogen = instrument.read_register(4, 0)    # Register 0x0004 = 4
                phosphorus = instrument.read_register(5, 0)  # Register 0x0005 = 5
                potassium = instrument.read_register(6, 0)   # Register 0x0006 = 6
                
                print(f"M: {moisture:5.1f}% | T: {temperature:5.1f}°C | EC: {conductivity:5d} | PH: {ph:4.1f} | N: {nitrogen:4d} | P: {phosphorus:4d} | K: {potassium:4d}", end='\r')
                
                time.sleep(3)
                
            except Exception as e:
                print(f"\nError: {e}")
                time.sleep(5)
                
    except KeyboardInterrupt:
        print("\nStopped")

if __name__ == "__main__":
    # Try the bulk read method first (more efficient)
    read_7in1_sensor()
    
    # If bulk read doesn't work, uncomment the line below to try individual reads
    # read_individual_parameters()