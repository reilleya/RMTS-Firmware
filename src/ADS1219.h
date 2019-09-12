#include <i2c_t3.h>
#include <i2c_t3.h>
#include <arduino.h>

#define CONFIG_READ_DUCER 0b10100101
#define CONFIG_READ_LC 0b01100101
#define CONFIG_ADDRESS 0x40

class ADS1219 {
    public:
        ADS1219(uint8_t address, uint8_t dataReadyPin, uint8_t ampPin);
        void setup();
        void writeRegister(uint8_t value);
        void requestReading();
        uint32_t getReading();
        uint32_t waitForReading();
    
    private:
        uint8_t addr;
        uint8_t drdy;
        uint8_t amp;
};