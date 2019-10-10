// Pins and addresses
#define ADC_ADDRESS 0x40
#define ADC_DRDY_PIN 19
#define ADC_AMP_PIN 18

#define PYRO_FIRE_PIN 5
#define PYRO_CONT_PIN 21
#define PYRO_FIRE_DURATION 100

// States
typedef enum STATE {
    SETUP,
    FIRING,
    FINISHED,
    ERROR
} systemState;

