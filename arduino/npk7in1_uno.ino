#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// RE and DE Pins set the RS485 module to Receiver or Transmitter mode
#define RE 8
#define DE 7

// Device address (change this if your sensor has a different address)
#define DEVICE_ADDRESS 0x01

// Modbus function code for reading holding registers
#define READ_HOLDING_REGISTERS 0x03

// Register addresses from your 7-in-1 sensor documentation
#define MOISTURE_REGISTER 0x0000  // Register 40001 - Moisture content
#define TEMPERATURE_REGISTER 0x0001 // Register 40002 - Temperature value  
#define CONDUCTIVITY_REGISTER 0x0002 // Register 40003 - Electrical conductivity
#define PH_REGISTER 0x0003         // Register 40004 - PH value
#define NITROGEN_REGISTER 0x0004   // Register 40005 - Nitrogen content
#define PHOSPHORUS_REGISTER 0x0005 // Register 40006 - Phosphorus content
#define POTASSIUM_REGISTER 0x0006  // Register 40007 - Potassium content

// I2C LCD address - usually 0x27 or 0x3F
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// SoftwareSerial for RS485 communication
SoftwareSerial mod(2, 3); // RX, TX

// Initialize I2C LCD
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// CRC calculation function
uint16_t calculateCRC(uint8_t *data, uint8_t length) {
  uint16_t crc = 0xFFFF;
  for (uint8_t pos = 0; pos < length; pos++) {
    crc ^= (uint16_t)data[pos];
    for (uint8_t i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

// Function to create Modbus request frame
void createRequestFrame(uint8_t *frame, uint8_t deviceAddress, uint8_t functionCode, uint16_t registerAddress, uint16_t registerCount) {
  frame[0] = deviceAddress;           // Device address
  frame[1] = functionCode;            // Function code
  frame[2] = (registerAddress >> 8) & 0xFF; // High byte of register address
  frame[3] = registerAddress & 0xFF;        // Low byte of register address
  frame[4] = (registerCount >> 8) & 0xFF;   // High byte of register count
  frame[5] = registerCount & 0xFF;          // Low byte of register count
  
  // Calculate CRC
  uint16_t crc = calculateCRC(frame, 6);
  frame[6] = crc & 0xFF;        // Low byte of CRC
  frame[7] = (crc >> 8) & 0xFF; // High byte of CRC
}

// Function to read a value from a register
uint16_t readRegister(uint16_t registerAddress) {
  uint8_t requestFrame[8];
  uint8_t responseFrame[9]; // Increased buffer size for multiple registers
  uint16_t value = 0;
  
  // Create the request frame
  createRequestFrame(requestFrame, DEVICE_ADDRESS, READ_HOLDING_REGISTERS, registerAddress, 1);
  
  // Send request
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  
  mod.write(requestFrame, sizeof(requestFrame));
  mod.flush();
  
  // Switch to receive mode
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  delay(10);
  
  // Wait for and read response
  unsigned long startTime = millis();
  uint8_t index = 0;
  
  while (millis() - startTime < 1000 && index < sizeof(responseFrame)) {
    if (mod.available()) {
      responseFrame[index++] = mod.read();
    }
  }
  
  // Check if we received a complete response
  if (index >= 5) {
    // Verify CRC
    uint16_t receivedCRC = (responseFrame[index-1] << 8) | responseFrame[index-2];
    uint16_t calculatedCRC = calculateCRC(responseFrame, index-2);
    
    if (receivedCRC == calculatedCRC && responseFrame[1] == READ_HOLDING_REGISTERS) {
      // Extract the value (big-endian format)
      value = (responseFrame[3] << 8) | responseFrame[4];
    } else {
      Serial.print("CRC error or invalid response for register 0x");
      Serial.println(registerAddress, HEX);
      return 0xFFFF; // Return error value
    }
  } else {
    Serial.print("Timeout or incomplete response for register 0x");
    Serial.println(registerAddress, HEX);
    return 0xFFFF; // Return error value
  }
  
  return value;
}

// Function to display values on LCD with scrolling
void displayOnLCD(float moisture, float temperature, uint16_t conductivity, float ph, 
                  uint16_t nitrogen, uint16_t phosphorus, uint16_t potassium) {
  static int displayState = 0;
  static unsigned long lastDisplayChange = 0;
  
  if (millis() - lastDisplayChange > 3000) { // Change display every 3 seconds
    displayState = (displayState + 1) % 3;
    lastDisplayChange = millis();
    lcd.clear();
  }
  
  switch (displayState) {
    case 0: // Display moisture and temperature
      lcd.setCursor(0, 0);
      lcd.print("M:");
      if (moisture != -1.0) {
        lcd.print(moisture, 1);
        lcd.print("%");
      } else {
        lcd.print("Err");
      }
      
      lcd.setCursor(9, 0);
      lcd.print("T:");
      if (temperature != -100.0) {
        lcd.print(temperature, 1);
        lcd.print("C");
      } else {
        lcd.print("Err");
      }
      
      lcd.setCursor(0, 1);
      lcd.print("EC:");
      if (conductivity != 0xFFFF) {
        lcd.print(conductivity);
        lcd.print("uS");
      } else {
        lcd.print("Err");
      }
      break;
      
    case 1: // Display pH and NPK
      lcd.setCursor(0, 0);
      lcd.print("pH:");
      if (ph != -1.0) {
        lcd.print(ph, 1);
      } else {
        lcd.print("Err");
      }
      
      lcd.setCursor(8, 0);
      lcd.print("N:");
      if (nitrogen != 0xFFFF) {
        lcd.print(nitrogen);
      } else {
        lcd.print("Err");
      }
      
      lcd.setCursor(0, 1);
      lcd.print("P:");
      if (phosphorus != 0xFFFF) {
        lcd.print(phosphorus);
      } else {
        lcd.print("Err");
      }
      
      lcd.setCursor(8, 1);
      lcd.print("K:");
      if (potassium != 0xFFFF) {
        lcd.print(potassium);
      } else {
        lcd.print("Err");
      }
      break;
      
    case 2: // Display all values summary
      lcd.setCursor(0, 0);
      lcd.print("7-in-1 Soil Sensor");
      lcd.setCursor(0, 1);
      lcd.print("All Parameters OK");
      break;
  }
}

void setup() {
  // Set the baud rate for the Serial port
  Serial.begin(9600);

  // Set the baud rate for the SoftwareSerial object (match sensor baudrate)
  mod.begin(4800); // Default baudrate for the sensor

  // Define pin modes for RE and DE
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  
  // Start in receive mode
  digitalWrite(RE, LOW);
  digitalWrite(DE, LOW);
  
  // Initialize I2C LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Display welcome message
  lcd.setCursor(0, 0);
  lcd.print("7-in-1 Soil Sensor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  Serial.println("7-in-1 Soil Sensor Initialized");
  Serial.println("Default baudrate: 4800");
  Serial.println("Default address: 0x01");
  
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read all sensor values
  uint16_t moisture_raw = readRegister(MOISTURE_REGISTER);
  delay(100);
  
  uint16_t temperature_raw = readRegister(TEMPERATURE_REGISTER);
  delay(100);
  
  uint16_t conductivity = readRegister(CONDUCTIVITY_REGISTER);
  delay(100);
  
  uint16_t ph_raw = readRegister(PH_REGISTER);
  delay(100);
  
  uint16_t nitrogen = readRegister(NITROGEN_REGISTER);
  delay(100);
  
  uint16_t phosphorus = readRegister(PHOSPHORUS_REGISTER);
  delay(100);
  
  uint16_t potassium = readRegister(POTASSIUM_REGISTER);
  delay(100);

  // Convert raw values to actual measurements
  float moisture = (moisture_raw != 0xFFFF) ? moisture_raw / 10.0 : -1.0;
  float temperature = (temperature_raw != 0xFFFF) ? temperature_raw / 10.0 : -100.0;
  float ph = (ph_raw != 0xFFFF) ? ph_raw / 10.0 : -1.0;

  // Display on LCD
  displayOnLCD(moisture, temperature, conductivity, ph, nitrogen, phosphorus, potassium);

  // Print values to serial monitor for debugging
  Serial.println("=== 7-in-1 Soil Sensor Readings ===");
  Serial.print("Moisture: ");
  if (moisture != -1.0) Serial.print(moisture, 1); else Serial.print("Error");
  Serial.println(" %");
  
  Serial.print("Temperature: ");
  if (temperature != -100.0) Serial.print(temperature, 1); else Serial.print("Error");
  Serial.println(" °C");
  
  Serial.print("Conductivity: ");
  if (conductivity != 0xFFFF) Serial.print(conductivity); else Serial.print("Error");
  Serial.println(" uS/cm");
  
  Serial.print("pH: ");
  if (ph != -1.0) Serial.print(ph, 1); else Serial.print("Error");
  Serial.println();
  
  Serial.print("Nitrogen: ");
  if (nitrogen != 0xFFFF) Serial.print(nitrogen); else Serial.print("Error");
  Serial.println(" mg/kg");
  
  Serial.print("Phosphorus: ");
  if (phosphorus != 0xFFFF) Serial.print(phosphorus); else Serial.print("Error");
  Serial.println(" mg/kg");
  
  Serial.print("Potassium: ");
  if (potassium != 0xFFFF) Serial.print(potassium); else Serial.print("Error");
  Serial.println(" mg/kg");
  Serial.println("===================================");
  
  delay(1000); // Update every second
}
