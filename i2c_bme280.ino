#include <Wire.h>
#include "BME280.h"

// BME280 dengan alamat 0x76 yang sudah pasti
BME280 bme(0x76);

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("=== BME280 Test - Address 0x76 ===");
    
    // Step 1: Test I2C communication
    Serial.println("\nStep 1: Testing I2C communication...");
    Wire.begin();
    delay(100);
    
    Wire.beginTransmission(0x76);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
        Serial.println("✓ I2C communication OK");
    } else {
        Serial.print("❌ I2C Error code: ");
        Serial.println(error);
        Serial.println("Check wiring!");
        while(1); // Stop here
    }
    
    // Step 2: Read Chip ID manually
    Serial.println("\nStep 2: Reading Chip ID manually...");
    Wire.beginTransmission(0x76);
    Wire.write(0xD0); // ID register
    Wire.endTransmission();
    
    Wire.requestFrom((uint8_t)0x76, (uint8_t)1);
    if (Wire.available()) {
        uint8_t chipID = Wire.read();
        Serial.print("Chip ID: 0x");
        Serial.println(chipID, HEX);
        
        if (chipID == 0x60) {
            Serial.println("✓ BME280 detected!");
        } else if (chipID == 0x58) {
            Serial.println("⚠️  BMP280 detected (no humidity sensor)");
        } else if (chipID == 0xFF || chipID == 0x00) {
            Serial.println("❌ No sensor response or wiring issue");
            while(1);
        } else {
            Serial.println("❌ Unknown sensor");
            while(1);
        }
    } else {
        Serial.println("❌ No data received from sensor");
        while(1);
    }
    
    // Step 3: Test dengan library
    Serial.println("\nStep 3: Testing with library...");
    if (bme.begin()) {
        Serial.println("✓ BME280 library initialization successful!");
        
        // Test first reading
        Serial.println("\nStep 4: First sensor reading...");
        testSensorReading();
        
    } else {
        Serial.println("❌ BME280 library initialization failed!");
        
        // Debug library initialization
        debugLibraryInit();
    }
}

void loop() {
    // Continuous readings if initialization was successful
    static unsigned long lastReading = 0;
    
    if (millis() - lastReading > 2000) {
        float temp = bme.readTemperature();
        float press = bme.readPressure();
        float hum = bme.readHumidity();
        
        Serial.println("\n--- Sensor Reading ---");
        Serial.print("Temperature: "); Serial.print(temp); Serial.println(" °C");
        Serial.print("Pressure: "); Serial.print(press/100.0); Serial.println(" hPa");
        Serial.print("Humidity: "); Serial.print(hum); Serial.println(" %");
        
        lastReading = millis();
    }
}

void testSensorReading() {
    delay(100); // Wait for sensor to be ready
    
    float temp = bme.readTemperature();
    float press = bme.readPressure();
    float hum = bme.readHumidity();
    
    Serial.print("Temperature: "); Serial.print(temp); Serial.println(" °C");
    Serial.print("Pressure: "); Serial.print(press/100.0); Serial.println(" hPa");
    Serial.print("Humidity: "); Serial.print(hum); Serial.println(" %");
    
    // Validate readings
    if (temp > -40 && temp < 85) {
        Serial.println("✓ Temperature reading valid");
    } else {
        Serial.println("⚠️  Temperature reading suspicious");
    }
    
    if (press > 30000 && press < 110000) {
        Serial.println("✓ Pressure reading valid");
    } else {
        Serial.println("⚠️  Pressure reading suspicious");
    }
    
    if (hum >= 0 && hum <= 100) {
        Serial.println("✓ Humidity reading valid");
    } else {
        Serial.println("⚠️  Humidity reading suspicious");
    }
}

void debugLibraryInit() {
    Serial.println("\n--- Debug Library Initialization ---");
    
    // Manual step-by-step initialization
    Serial.println("Manual initialization steps:");
    
    // 1. Read ID again
    Wire.beginTransmission(0x76);
    Wire.write(0xD0);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)0x76, (uint8_t)1);
    uint8_t id = Wire.read();
    Serial.print("1. ID Register: 0x"); Serial.println(id, HEX);
    
    // 2. Reset
    Serial.println("2. Sending reset command...");
    Wire.beginTransmission(0x76);
    Wire.write(0xE0); // Reset register
    Wire.write(0xB6); // Reset value
    Wire.endTransmission();
    delay(100);
    
    // 3. Check ID after reset
    Wire.beginTransmission(0x76);
    Wire.write(0xD0);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)0x76, (uint8_t)1);
    id = Wire.read();
    Serial.print("3. ID after reset: 0x"); Serial.println(id, HEX);
    
    // 4. Read status register
    Wire.beginTransmission(0x76);
    Wire.write(0xF3); // Status register
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)0x76, (uint8_t)1);
    uint8_t status = Wire.read();
    Serial.print("4. Status register: 0x"); Serial.println(status, HEX);
    
    // 5. Try to read calibration data
    Serial.println("5. Reading first calibration bytes...");
    Wire.beginTransmission(0x76);
    Wire.write(0x88); // First calibration register
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)0x76, (uint8_t)6);
    
    Serial.print("   Calib data: ");
    for (int i = 0; i < 6 && Wire.available(); i++) {
        uint8_t data = Wire.read();
        Serial.print("0x"); 
        if (data < 16) Serial.print("0");
        Serial.print(data, HEX); Serial.print(" ");
    }
    Serial.println();
}

