#include "common.h"

extern timer_instance_t g_timer;

void usdelay(uint32_t us) {
    uint32_t t = (float)(us) * (float)TIMER_LOAD_VALUE;
    TMR_reload(&g_timer,  t);
    TMR_start(&g_timer);
    while(TMR_current_value(&g_timer) > 0);
    TMR_stop(&g_timer);
}
