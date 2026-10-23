#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* AXP2101 observations. Percentage is the PMIC's estimate, never a voltage
   interpolation. Unknown, absent battery and stale data are distinct. */
typedef struct {
    uint32_t sampled_at;
    unsigned percent,millivolts;
    bool available,battery,vbus,charging,percent_valid,voltage_valid;
} pd_power_info;
typedef bool (*pd_power_reader)(uint8_t reg,uint8_t *out,size_t count,void *context);
typedef bool (*pd_power_writer)(uint8_t reg,uint8_t value,void *context);
/* Read-only sampling. Required-read failure clears availability; optional
   gauge/ADC failures only remove that measurement. No charger configuration. */
bool pd_power_sample(pd_power_info *out,uint32_t now,pd_power_reader read,void *context);
bool pd_power_fresh(const pd_power_info *info,uint32_t now);
unsigned pd_power_warning(const pd_power_info *info,uint32_t now); /* 0, low=1 (<=20%), critical=2 (<=5%). */
bool pd_power_battery_only(const pd_power_info *info,uint32_t now);
bool pd_power_auto_pause(const pd_power_info *info,uint32_t now,uint32_t activity,bool enabled,bool blocked);
/* Explicit shutdown only. Rechecks identity, battery/VBUS and register 0x10;
   sets PWROFF, clears command bit RESET, preserves configuration bits. */
bool pd_power_off(pd_power_reader read,pd_power_writer write,void *context);
