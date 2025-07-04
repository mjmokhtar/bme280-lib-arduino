#include "BME280.h"
#include <math.h>

BME280::BME280(uint8_t address) {
    _address = address;
    _t_fine = 0;
}

bool BME280::begin() {
    Wire.begin();
    delay(100); // Berikan waktu untuk I2C stabil
    
    // Test I2C communication first
    Wire.beginTransmission(_address);
    if (Wire.endTransmission() != 0) {
        Serial.print("I2C Error: No device at address 0x");
        Serial.println(_address, HEX);
        return false;
    }
    
    // Check chip ID
    uint8_t chipID = getChipID();
    Serial.print("Chip ID read: 0x");
    Serial.println(chipID, HEX);
    
    if (chipID != BME280_CHIP_ID) {
        Serial.print("Wrong chip ID! Expected: 0x");
        Serial.print(BME280_CHIP_ID, HEX);
        Serial.print(", Got: 0x");
        Serial.println(chipID, HEX);
        return false;
    }
    
    Serial.println("BME280 chip ID correct");
    
    // Reset sensor
    Serial.println("Resetting sensor...");
    reset();
    delay(100); // Tunggu reset selesai
    
    // Verify sensor is responsive after reset
    chipID = getChipID();
    if (chipID != BME280_CHIP_ID) {
        Serial.println("Sensor not responsive after reset");
        return false;
    }
    
    // Read calibration data
    Serial.println("Reading calibration data...");
    readCalibrationData();
    
    // Set default configuration
    Serial.println("Setting configuration...");
    setOversamplingTemperature(BME280_OVERSAMP_16X);
    setOversamplingPressure(BME280_OVERSAMP_16X);
    setOversamplingHumidity(BME280_OVERSAMP_16X);
    setFilter(BME280_FILTER_16);
    setStandbyTime(BME280_STANDBY_0_5);
    setMode(BME280_MODE_NORMAL);
    
    Serial.println("BME280 initialization complete");
    return true;
}

bool BME280::isConnected() {
    uint8_t chipID = getChipID();
    Serial.print("Reading chip ID: 0x");
    Serial.println(chipID, HEX);
    return (chipID == BME280_CHIP_ID);
}

uint8_t BME280::getChipID() {
    return readRegister(BME280_REG_ID);
}

void BME280::reset() {
    writeRegister(BME280_REG_RESET, 0xB6);
}

uint8_t BME280::readRegister(uint8_t reg) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    byte error = Wire.endTransmission();
    
    if (error != 0) {
        return 0; // I2C error
    }
    
    Wire.requestFrom(_address, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0;
}

void BME280::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
    delay(1); // Small delay after write
}

void BME280::readCalibrationData() {
    // Read temperature and pressure calibration data (0x88-0x9F)
    Wire.beginTransmission(_address);
    Wire.write(BME280_REG_CALIB00);
    Wire.endTransmission();
    
    Wire.requestFrom(_address, (uint8_t)24);
    
    _calibData.dig_T1 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_T2 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_T3 = (Wire.read() << 0) | (Wire.read() << 8);
    
    _calibData.dig_P1 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_P2 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_P3 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_P4 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_P5 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_P6 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_P7 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_P8 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_P9 = (Wire.read() << 0) | (Wire.read() << 8);
    
    Wire.read(); // Skip 0x9A
    _calibData.dig_H1 = Wire.read();
    
    // Read humidity calibration data (0xE1-0xE7)
    Wire.beginTransmission(_address);
    Wire.write(BME280_REG_CALIB26);
    Wire.endTransmission();
    
    Wire.requestFrom(_address, (uint8_t)7);
    
    _calibData.dig_H2 = (Wire.read() << 0) | (Wire.read() << 8);
    _calibData.dig_H3 = Wire.read();
    
    uint8_t e4 = Wire.read();
    uint8_t e5 = Wire.read();
    uint8_t e6 = Wire.read();
    uint8_t e7 = Wire.read();
    
    _calibData.dig_H4 = (e4 << 4) | (e5 & 0x0F);
    _calibData.dig_H5 = (e6 << 4) | (e5 >> 4);
    _calibData.dig_H6 = e7;
}

void BME280::setMode(uint8_t mode) {
    uint8_t ctrlMeas = readRegister(BME280_REG_CTRL_MEAS);
    ctrlMeas = (ctrlMeas & 0xFC) | (mode & 0x03);
    writeRegister(BME280_REG_CTRL_MEAS, ctrlMeas);
}

void BME280::setOversamplingTemperature(uint8_t oversampling) {
    uint8_t ctrlMeas = readRegister(BME280_REG_CTRL_MEAS);
    ctrlMeas = (ctrlMeas & 0x1F) | ((oversampling & 0x07) << 5);
    writeRegister(BME280_REG_CTRL_MEAS, ctrlMeas);
}

void BME280::setOversamplingPressure(uint8_t oversampling) {
    uint8_t ctrlMeas = readRegister(BME280_REG_CTRL_MEAS);
    ctrlMeas = (ctrlMeas & 0xE3) | ((oversampling & 0x07) << 2);
    writeRegister(BME280_REG_CTRL_MEAS, ctrlMeas);
}

void BME280::setOversamplingHumidity(uint8_t oversampling) {
    writeRegister(BME280_REG_CTRL_HUM, oversampling & 0x07);
    // Changes to ctrl_hum only become effective after a write to ctrl_meas
    uint8_t ctrlMeas = readRegister(BME280_REG_CTRL_MEAS);
    writeRegister(BME280_REG_CTRL_MEAS, ctrlMeas);
}

void BME280::setFilter(uint8_t filter) {
    uint8_t config = readRegister(BME280_REG_CONFIG);
    config = (config & 0xE3) | ((filter & 0x07) << 2);
    writeRegister(BME280_REG_CONFIG, config);
}

void BME280::setStandbyTime(uint8_t standby) {
    uint8_t config = readRegister(BME280_REG_CONFIG);
    config = (config & 0x1F) | ((standby & 0x07) << 5);
    writeRegister(BME280_REG_CONFIG, config);
}

bool BME280::isMeasuring() {
    uint8_t status = readRegister(BME280_REG_STATUS);
    return (status & 0x08) != 0;
}

int32_t BME280::readTemperatureRaw() {
    uint8_t msb = readRegister(BME280_REG_TEMP_MSB);
    uint8_t lsb = readRegister(BME280_REG_TEMP_LSB);
    uint8_t xlsb = readRegister(BME280_REG_TEMP_XLSB);
    
    return ((uint32_t)msb << 12) | ((uint32_t)lsb << 4) | ((xlsb >> 4) & 0x0F);
}

int32_t BME280::readPressureRaw() {
    uint8_t msb = readRegister(BME280_REG_PRESS_MSB);
    uint8_t lsb = readRegister(BME280_REG_PRESS_LSB);
    uint8_t xlsb = readRegister(BME280_REG_PRESS_XLSB);
    
    return ((uint32_t)msb << 12) | ((uint32_t)lsb << 4) | ((xlsb >> 4) & 0x0F);
}

int32_t BME280::readHumidityRaw() {
    uint8_t msb = readRegister(BME280_REG_HUM_MSB);
    uint8_t lsb = readRegister(BME280_REG_HUM_LSB);
    
    return ((uint32_t)msb << 8) | lsb;
}

float BME280::readTemperature() {
    int32_t adc_T = readTemperatureRaw();
    
    // Temperature compensation formula from datasheet
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)_calibData.dig_T1 << 1))) * 
                    ((int32_t)_calibData.dig_T2)) >> 11;
    
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)_calibData.dig_T1)) * 
                      ((adc_T >> 4) - ((int32_t)_calibData.dig_T1))) >> 12) * 
                    ((int32_t)_calibData.dig_T3)) >> 14;
    
    _t_fine = var1 + var2;
    
    float temperature = (_t_fine * 5 + 128) >> 8;
    return temperature / 100.0;
}

float BME280::readPressure() {
    // Read temperature first to get t_fine
    readTemperature();
    
    int32_t adc_P = readPressureRaw();
    
    // Pressure compensation formula from datasheet
    int64_t var1 = ((int64_t)_t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)_calibData.dig_P6;
    var2 = var2 + ((var1 * (int64_t)_calibData.dig_P5) << 17);
    var2 = var2 + (((int64_t)_calibData.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)_calibData.dig_P3) >> 8) + 
           ((var1 * (int64_t)_calibData.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)_calibData.dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0; // Avoid division by zero
    }
    
    int64_t pressure = 1048576 - adc_P;
    pressure = (((pressure << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)_calibData.dig_P9) * (pressure >> 13) * (pressure >> 13)) >> 25;
    var2 = (((int64_t)_calibData.dig_P8) * pressure) >> 19;
    
    pressure = ((pressure + var1 + var2) >> 8) + (((int64_t)_calibData.dig_P7) << 4);
    
    return pressure / 256.0;
}

float BME280::readHumidity() {
    // Read temperature first to get t_fine
    readTemperature();
    
    int32_t adc_H = readHumidityRaw();
    
    // Humidity compensation formula from datasheet
    int32_t var1 = (_t_fine - ((int32_t)76800));
    
    var1 = (((((adc_H << 14) - (((int32_t)_calibData.dig_H4) << 20) - 
              (((int32_t)_calibData.dig_H5) * var1)) + ((int32_t)16384)) >> 15) * 
            (((((((var1 * ((int32_t)_calibData.dig_H6)) >> 10) * 
                (((var1 * ((int32_t)_calibData.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + 
              ((int32_t)2097152)) * ((int32_t)_calibData.dig_H2) + 8192) >> 14));
    
    var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * 
                     ((int32_t)_calibData.dig_H1)) >> 4));
    
    var1 = (var1 < 0 ? 0 : var1);
    var1 = (var1 > 419430400 ? 419430400 : var1);
    
    float humidity = (var1 >> 12);
    return humidity / 1024.0;
}

float BME280::readAltitude(float seaLevelPressure) {
    float pressure = readPressure() / 100.0; // Convert to hPa
    return 44330.0 * (1.0 - pow(pressure / seaLevelPressure, 0.1903));
}