//TODO:
//CHECK IF THE CLOCK FREQUENCY IS REAL WITH OSCILLOSCOPE

//
// Created by marijn on 10/14/25.
//
#include <libpynq.h>
#include <time.h>


#define MOTOR_DRIVER_SM_ADDR        0x51U

#define MOTOR_DRIVER_SM_RMAP_SIZE    0x02U

#define CLOCK_FREQ                   100000000
#define PWM_FREQ                     1000
#define AMP_CHANNEL                  PWM0
#define FREQ_CHANNEL                 PWM1
uint32_t rmap[MOTOR_DRIVER_SM_RMAP_SIZE] = {
    0x00000000,        // amplitude
    0x00000000,        // frequency
};

#define MOTOR_DRIVER_SM_AMPLITUDE_REG    0x00U
#define MOTOR_DRIVER_SM_FREQUENCY_REG    0x01U


display_t display;
FontxFile fx;

void write_to_screen(uint32_t motor_amplitude, uint32_t motor_frequency)
{
    // Clear display
    displayFillScreen(&display, RGB_BLACK);

    int y = 20;
    int line_height = 20;

    // Motor amplitude
    char amplitude_string[50];
    snprintf(amplitude_string, sizeof(amplitude_string), "motor amp: %lu", (unsigned long)motor_amplitude);
    displayDrawString(&display, &fx, 10, y, (uint8_t *)amplitude_string, RGB_GREEN);

    // Motor frequency
    y += line_height;
    char frequency_string[50];
    snprintf(frequency_string, sizeof(frequency_string), "motor freq: %lu", (unsigned long)motor_frequency);
    displayDrawString(&display, &fx, 10, y, (uint8_t *)frequency_string, RGB_GREEN);
}


int main(void) {
    // init
    pynq_init();
    switchbox_init();

    // pins
    switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
    switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);
    switchbox_set_pin(IO_AR0, SWB_PWM0);
    switchbox_set_pin(IO_AR1, SWB_PWM1);

    //PWM
    pwm_init(AMP_CHANNEL, CLOCK_FREQ/PWM_FREQ);
    pwm_init(FREQ_CHANNEL, CLOCK_FREQ/PWM_FREQ);
    float PWM_STATES[5] = {0.95, 0.8, 0.6, 0.4, 0.2};

    // I2C
    iic_init(IIC0);
    iic_reset(IIC0);
    iic_set_slave_mode(IIC0, MOTOR_DRIVER_SM_ADDR, &(rmap[0]), MOTOR_DRIVER_SM_RMAP_SIZE);

    InitFontx(&fx, "../../fonts/ILGH24XB.FNT", "");
	display_init(&display);
    displayFillScreen(&display, RGB_BLACK);
   

    // loop
    for (;;) {
        // handle I2C
        iic_slave_mode_handler(IIC0);

        //set pwm
        pwm_set_duty_cycle(AMP_CHANNEL, (uint32_t)(PWM_STATES[rmap[0]-1]*CLOCK_FREQ/PWM_FREQ));
        pwm_set_duty_cycle(FREQ_CHANNEL, (uint32_t)(PWM_STATES[rmap[1]-1]*CLOCK_FREQ/PWM_FREQ));

        // print data
        for (uint8_t i = 0; i < MOTOR_DRIVER_SM_RMAP_SIZE; i++) {
        printf("%u", rmap[i]);
        if (i != MOTOR_DRIVER_SM_RMAP_SIZE - 1) {
            printf(", ");
        }
        }
        printf("\n");
        write_to_screen(rmap[0], rmap[1]);
        sleep_msec(50);
    }


    // return
    iic_destroy(IIC0);
    pynq_destroy();
    return EXIT_SUCCESS;
}
