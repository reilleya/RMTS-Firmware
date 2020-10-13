// ====================================================================================================================
// Pins and addresses
// ====================================================================================================================

#define ADC_ADDRESS 0x40
#define ADC_DRDY_PIN 19
#define ADC_AMP_PIN 18

#define PYRO_FIRE_PIN 5
#define PYRO_CONT_PIN 21

#define SD_MOSI 11
#define SD_SCK 13
#define SD_MISO 12
#define SD_CS 10

#define STATUS_PIN 31
#define TRIGGER_PIN 28


// ====================================================================================================================
// States
// ====================================================================================================================

typedef enum STATE {
    SETUP,
    FIRING,
    FINISHED,
    ERROR
} systemState;


// ====================================================================================================================
// Errors
// ====================================================================================================================

// No SD error
#define ERROR_SD_OK 0
// No SD card detected
#define ERROR_SD_MISSING 1
// Unable to create a file on the SD card
#define ERROR_SD_UNWRITABLE 2
// More than 256 log files already exist on the card
#define ERROR_SD_TOO_MANY_LOGS 3
#define ERROR_SD_UNINITIALIZED 255

// No ADC error
#define ERROR_ADC_OK 0
// Writing to a register on the ADC and reading it back gave the wrong result
#define ERROR_ADC_SELF_CHECK 1
#define ERROR_ADC_UNINITIALIZED 255

#define ERROR_RADIO_OK 0
#define ERROR_RADIO_BAD_FIRE 1


// ====================================================================================================================
// Packet types
// ====================================================================================================================

#define PACKET_SETUP 0
#define PACKET_ERROR 1
#define PACKET_RESULTS 2
#define PACKET_VERSION 3
#define PACKET_FIRING 4

#define PACKET_FIRE 128
#define PACKET_STOP 129
#define PACKET_CAL_START 130
#define PACKET_CAL_STOP 131


// ====================================================================================================================
// Firing state constants
// ====================================================================================================================

// The number of frames recorded at startup to tare transducers
#define NUM_CAL_FRAMES 10


// ====================================================================================================================
// Results state constants
// ====================================================================================================================

// The number of packets skipped when sending results. 0->10->20 ... 1->11->21
#define RESULTS_STRIDE 10
// When the results stage hits the end of the record, it adds this to the index of the point it started on, mods by 
// RESULTS_STRIDE, and starts sending again from that point. Should not divide evenly into RESULTS_STRIDE.
#define RESULTS_OFFSET_INCREMENT 7
// Every N frames, send a version packet
#define RESULTS_VERSION_PERIOD 30


// ====================================================================================================================
// Blink patterns
// ====================================================================================================================

// Indicator stays off
#define PATTERN_OFF {0, 100000, 1, 0}
// Indicator stays on
#define PATTERN_ON {100000, 0, 1, 0}
// Indicator repeats two quick blinks
#define PATTERN_SETUP {200, 2000, 2, 200}
// Indicator flashes on and off rapidly
#define PATTERN_ERROR {75, 75, 1, 0}
// Indicator flashes on and off slowly
#define PATTERN_FINISHED {750, 750, 1, 0}


// ====================================================================================================================
// Versioning
// ====================================================================================================================

// This should eventually be determined by the hardware itself but use a constant for now
#define HARDWARE_VERSION 2
// Increment this for any firmware "release"
#define FIRMWARE_VERSION 5
