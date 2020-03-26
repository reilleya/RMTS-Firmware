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

#define STATUS_PIN 31

// States
typedef enum STATE {
    SETUP,
    FIRING,
    FINISHED,
    ERROR
} systemState;


// Errors
#define ERROR_SD_OK 0 // No SD error
#define ERROR_SD_MISSING 1 // No SD card detected
#define ERROR_SD_UNWRITABLE 2 // Unable to create a file on the SD card
#define ERROR_SD_TOO_MANY_LOGS 3 // More than 256 log files already exist on the card
#define ERROR_SD_UNINITIALIZED 255

#define ERROR_ADC_OK 0 // No ADC error
#define ERROR_ADC_SELF_CHECK 1 // Writing to a register on the ADC and reading it back gave the wrong result
#define ERROR_ADC_UNINITIALIZED 255

#define ERROR_RADIO_OK 0
#define ERROR_RADIO_BAD_FIRE 1


// Packets
#define PACKET_SETUP 0
#define PACKET_ERROR 1
#define PACKET_RESULTS 2

#define PACKET_FIRE 128
#define PACKET_STOP 129
#define PACKET_CAL_START 130
#define PACKET_CAL_STOP 131


// The number of frames recorded at startup to tare transducers
#define NUM_CAL_FRAMES 10

// The number of packets skipped when sending results. 0->10->20 ... 1->11->21
#define RESULTS_STRIDE 10

// Blink Patterns
#define PATTERN_OFF {0, 100000, 1, 0} // Indicator stays off
#define PATTERN_SETUP {200, 2000, 2, 200} // Indicator repeats two quick blinks
#define PATTERN_ERROR {75, 75, 1, 0} // Indicator flashes on and off rapidly
#define PATTERN_FINISHED {750, 750, 1, 0} // Indicator flashes on and off slowly
