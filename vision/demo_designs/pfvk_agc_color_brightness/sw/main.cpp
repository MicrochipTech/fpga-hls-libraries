#include "common.h"
#include "hls_accelerator_driver.h"

#ifdef USE_UART
UART_instance_t gUart;
uint8_t gTxBuf[UART_BUF_SZ];
#endif

i2c_instance_t g_i2c_instance_cam2;
gpio_instance_t g_gpio_out;
timer_instance_t g_timer;

int to_10bint(uint32_t x) {
    int ret = (x < 511) ? x : x - 1024;
    return ret;
}

#ifdef __cplusplus
extern "C" {
#endif

uint8_t MSYS_EI0_IRQHandler(void) {
    I2C_isr(&g_i2c_instance_cam2);
    return (EXT_IRQ_KEEP_ENABLED);
}

void I2C_enable_irq( i2c_instance_t * this_i2c ) {
    if(this_i2c == &g_i2c_instance_cam2) {
        MRV_enable_local_irq(MRV32_MSYS_EIE0_IRQn);
    } else {
        PRINTF("\r\nERROR!\r\n");
    }
}

void I2C_disable_irq( i2c_instance_t * this_i2c ) {
    if(this_i2c == &g_i2c_instance_cam2) {
        MRV_disable_local_irq(MRV32_MSYS_EIE0_IRQn);
    } else {
        PRINTF("\r\nERROR!\r\n");
    }
}

void msdelay(uint32_t ms) {
    uint32_t t = (float)(ms) * (float)TIMER_LOAD_VALUE / 1000.0;
    TMR_reload(&g_timer,  t);
    TMR_start(&g_timer);
    do { } while(TMR_current_value(&g_timer) > 0);
    TMR_stop(&g_timer);
}

#ifdef __cplusplus
} // extern "C"
#endif

//------------------------------------------------------------------------------
void gain_cal(uint32_t total_average, unsigned channel, uint16_t *gain) {
    const int16_t good_average=70;
    const int16_t hysteresis=4;
    const uint32_t min_gain = 5;
    const uint32_t max_gain = 150;
    int16_t step;
    if(total_average < (good_average - hysteresis)) {
        step = 1;
    } else {
        step = (total_average > (good_average + hysteresis)) ? -1 : 0;
    }

    *gain = *gain + step;

    if(*gain < min_gain) {
        *gain = min_gain;
    } else {
        if(*gain >= max_gain)
            *gain = max_gain;
    }

    PRINTF("avg:%d, gain:%d\r\n", total_average, *gain);
    gain_setting(channel, *gain);
}

//------------------------------------------------------------------------------
int main( void ) {
    #ifdef USE_UART
    UART_init( &gUart, COREUARTAPB0_BASE_ADDR, BAUD_VALUE_115200, 
        (DATA_8_BITS | NO_PARITY) );
    #endif

    // Escape sequence to clean the terminal and print the compilation time
    PRINTF("\x1B[2J File: %s\r\n", __TIME__); 

    // Enable interrupts from the CoreI2C associated to the camera
    MRV_enable_local_irq(MRV32_MSYS_EIE0_IRQn); // CAM2

    // Configure timer
    TMR_init(&g_timer, CORETIMER0_BASE_ADDR, TMR_ONE_SHOT_MODE,
        PRESCALER_DIV_2, TIMER_LOAD_VALUE);

    // Enable interrupts in general
    HAL_enable_interrupts();

    // Configure GPIO
    GPIO_init(&g_gpio_out, COREGPIO_OUT_BASE_ADDR, GPIO_APB_32_BITS_BUS);

    PRINTF("Initializing IMX334 sensor\r\n");
    imx334_cam_init();
    imx334_cam_reginit(2u);

    // Set color and brightness factors.  
    // Each channel in every pixel is divided by 32 in the ImageEnhance HLS module.
    // Therefore, a factor of 32 means no scaling for that channel. 
    VideoPipelineTop_write_b_const(62);      // 62/32 ~ 1.9
    VideoPipelineTop_write_g_const(42);      // 42/32 ~ 1.3
    VideoPipelineTop_write_r_const(52);      // 52/32 ~ 1.6
    VideoPipelineTop_write_brightness(12);   // Just a bit brighter
    VideoPipelineTop_write_enable_gamma(1);  // Enabled

    // No color processing...
    // VideoPipelineTop_write_b_const(32);     // 32/32 = 1
    // VideoPipelineTop_write_g_const(32);     // 32/32 = 1
    // VideoPipelineTop_write_r_const(32);     // 32/32 = 1
    // VideoPipelineTop_write_brightness(0);   // No additional brightness
    // VideoPipelineTop_write_enable_gamma(0); // No gamma correction


    uint16_t gain = 80; // initial cam gain

    while(1) {
        msdelay(100);

        // Read the sum of all pixels and compute the average. Remove the 
        // extra 2 factor added in the HLS module
        uint32_t sum;
        VideoPipelineTop_memcpy_read_sum((void*)(&sum), sizeof(sum));
        uint32_t avg = sum / (1920*1080*4*2);

        gain_cal(avg, 2, &gain);
    }
    return 0;
}