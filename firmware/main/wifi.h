#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "wifi_setup.h"
enum { PD_WIFI_OFF,PD_WIFI_CONNECTING,PD_WIFI_CONNECTED,PD_WIFI_FAILED };
enum { PD_WIFI_ENABLE=1,PD_WIFI_DISABLE,PD_WIFI_SETUP,PD_WIFI_FORGET,PD_WIFI_SCAN,PD_WIFI_SCAN_CANCEL };
enum { PD_WIFI_ERROR_NONE,PD_WIFI_ERROR_NETWORK,PD_WIFI_ERROR_STORAGE,PD_WIFI_ERROR_UNAVAILABLE };
typedef struct {
 unsigned state,error,setup_seconds;bool configured,setup,setup_saved,setup_failed,suspended,radio_active,joining;
 char ssid[33],ip[16],setup_ssid[33],setup_password[17];
} pd_wifi_info;
#define PD_WIFI_NETWORKS 16
enum { PD_WIFI_OPEN,PD_WIFI_PERSONAL,PD_WIFI_UNSUPPORTED };
typedef struct {char ssid[33];int8_t rssi;uint8_t security;} pd_wifi_network;
typedef struct {bool scanning,complete,truncated;unsigned count,error;pd_wifi_network networks[PD_WIFI_NETWORKS];} pd_wifi_scan_info;
/* Credentials never enter pd_state, character USB packets, diagnostics or backups. */
void pd_wifi_boot(void);
/* Serializes RNG with SAR/RF entropy handover. */
uint32_t pd_wifi_random(void);
void pd_wifi_snapshot(pd_wifi_info *out);
bool pd_wifi_command(unsigned command);
void pd_wifi_networks(pd_wifi_scan_info *out);
/* Queues an owned copy, never saves before matching DHCP; no PG/USB data. */
bool pd_wifi_join(const pd_wifi_credentials *credentials);
/* Pause powers the radio down; resume restores the prior desired STA state only. */
bool pd_wifi_pause(bool paused);
