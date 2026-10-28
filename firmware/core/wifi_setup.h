#pragma once
#include <stdbool.h>
#include <stddef.h>
typedef struct { char ssid[33],password[65]; bool open; } pd_wifi_credentials;
bool pd_wifi_credentials_valid(const pd_wifi_credentials *c);
/* Strict form parser: no duplicates, unknown fields, controls or overlong data.
   Output is unchanged on every failure. Token is per setup session, 32 hex bytes. */
bool pd_wifi_parse_form(const char *body,size_t size,const char *token,pd_wifi_credentials *out);
