// Pins and addresses
#define ADC_ADDRESS 0x40
#define ADC_DRDY_PIN 19
#define ADC_AMP_PIN 18

#define PYRO_FIRE_PIN 5
#define PYRO_CONT_PIN 21
#define PYRO_FIRE_DURATION 100

#define SD_MOSI 11
#define SD_SCK 13
#define SD_MISO 12
#define SD_CS 10

// States
typedef enum STATE {
    SETUP,
    FIRING,
    FINISHED,
    ERROR
} systemState;

// Errors

#define ERROR_SD_MISSING 1 // No SD card detected
#define ERROR_SD_UNWRITABLE 2 // Unable to create a file on the SD card
#define ERROR_SD_TOO_MANY_LOG 4 // More than 256 log files already exist on the card

#define ERROR_ADC_SELFCHECK 8 // Writing to a register on the ADC and reading it back gave the wrong result
