#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define PD_SYNC_CERT 1501
typedef struct {char id[33],host[16],character_id[16],token[65],certificate[PD_SYNC_CERT];uint32_t time,epoch,format;uint16_t port;} pd_sync_config;
bool pd_sync_config_parse(const char *json,size_t size,pd_sync_config *out);
