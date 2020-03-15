#include <i2c_t3.h>
#include <arduino.h>

// Read sensors at 20 Hz during setup for smoother data
#define CONFIG_READ_DUCER_SETUP 0b10100001
#define CONFIG_READ_LC_SETUP 0b01100001

// Read sensors at 300 Hz during actual data collection
#define CONFIG_READ_DUCER 0b10101001
#define CONFIG_READ_LC 0b01101001

#define CONFIG_ADDRESS 0x40

class ADS1219 {
    public:
        ADS1219(uint8_t address, uint8_t dataReadyPin, uint8_t ampPin);
        void setup();
        uint8_t getStatus();

        void writeRegister(uint8_t value);
        void requestReading();
        uint32_t getReading();
        uint32_t waitForReading();

        void startCal();
        void endCal();
    
        uint8_t drdy;

    private:
        uint8_t addr;
        uint8_t amp;

        uint8_t status;
};