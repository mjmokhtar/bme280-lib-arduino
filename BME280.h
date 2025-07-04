#ifndef BME280_H
#define BME280_H

#include <Arduino.h>
#include <Wire.h>

// BME280 I2C Addresses
#define BME280_ADDRESS_PRIMARY   0x76
#define BME280_ADDRESS_SECONDARY 0x77

// BME280 Register Addresses
#define BME280_REG_HUM_LSB       0xFE
#define BME280_REG_HUM_MSB       0xFD
#define BME280_REG_TEMP_XLSB     0xFC
#define BME280_REG_TEMP_LSB      0xFB
#define BME280_REG_TEMP_MSB      0xFA
#define BME280_REG_PRESS_XLSB    0xF9
#define BME280_REG_PRESS_LSB     0xF8
#define BME280_REG_PRESS_MSB     0xF7
#define BME280_REG_CONFIG        0xF5
#define BME280_REG_CTRL_MEAS     0xF4
#define BME280_REG_STATUS        0xF3
#define BME280_REG_CTRL_HUM      0xF2
#define BME280_REG_RESET         0xE0
// #define BME280_REG_ID            0x58
#define BME280_REG_ID            0xD0


// Calibration data registers
#define BME280_REG_CALIB00       0x88
#define BME280_REG_CALIB26       0xE1

// BME280 Chip ID
#define BME280_CHIP_ID           0x60

// Power modes
#define BME280_MODE_SLEEP        0x00
#define BME280_MODE_FORCED       0x01
#define BME280_MODE_NORMAL       0x03

// Oversampling settings
#define BME280_OVERSAMP_SKIP     0x00
#define BME280_OVERSAMP_1X       0x01
#define BME280_OVERSAMP_2X       0x02
#define BME280_OVERSAMP_4X       0x03
#define BME280_OVERSAMP_8X       0x04
#define BME280_OVERSAMP_16X      0x05

// Filter settings
#define BME280_FILTER_OFF        0x00
#define BME280_FILTER_2          0x01
#define BME280_FILTER_4          0x02
#define BME280_FILTER_8          0x03
#define BME280_FILTER_16         0x04

// Standby time settings
#define BME280_STANDBY_0_5       0x00
#define BME280_STANDBY_62_5      0x01
#define BME280_STANDBY_125       0x02
#define BME280_STANDBY_250       0x03
#define BME280_STANDBY_500       0x04
#define BME280_STANDBY_1000      0x05
#define BME280_STANDBY_10        0x06
#define BME280_STANDBY_20        0x07

// Calibration data structure
struct BME280_CalibData {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
};

class BME280 {
private:
    uint8_t _address;
    BME280_CalibData _calibData;
    int32_t _t_fine;  // Temperature fine value for pressure/humidity compensation
    
    // Private methods
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);
    void readCalibrationData();
    int32_t readTemperatureRaw();
    int32_t readPressureRaw();
    int32_t readHumidityRaw();
    
public:
    // Constructor
    BME280(uint8_t address = BME280_ADDRESS_PRIMARY);
    
    // Initialization
    bool begin();
    bool begin(uint8_t sda, uint8_t scl); // For ESP32/ESP8266 with custom pins
    bool isConnected();
    uint8_t getAddress(); // Get current I2C address
    
    // Configuration
    void setMode(uint8_t mode);
    void setOversamplingTemperature(uint8_t oversampling);
    void setOversamplingPressure(uint8_t oversampling);
    void setOversamplingHumidity(uint8_t oversampling);
    void setFilter(uint8_t filter);
    void setStandbyTime(uint8_t standby);
    
    // Reading data
    float readTemperature();     // Celsius
    float readPressure();        // Pascal
    float readHumidity();        // %RH
    float readAltitude(float seaLevelPressure = 1013.25);  // meters
    
    // Utility
    void reset();
    uint8_t getChipID();
    bool isMeasuring();
};

#endif