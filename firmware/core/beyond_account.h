#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define PD_ACCOUNT_COOKIE 4096
#define PD_ACCOUNT_TOKEN 16384
#define PD_ACCOUNT_ROWS 300
#define PD_ACCOUNT_LINE (PD_ACCOUNT_COOKIE+96)
typedef struct {uint32_t version;char user[16],cookie[PD_ACCOUNT_COOKIE+1];} pd_account_access;
typedef struct {char id[16],name[65],detail[97];unsigned level;} pd_account_row;
typedef struct {unsigned count;bool partial;pd_account_row rows[PD_ACCOUNT_ROWS];} pd_account_catalog;
enum {PD_ACCOUNT_HELLO=1,PD_ACCOUNT_OFFER,PD_ACCOUNT_CANCEL};
typedef struct {unsigned kind;char nonce[33];pd_account_access access;} pd_account_request;
void pd_account_wipe(void *p,size_t n);
bool pd_account_cookie(const char *p);
bool pd_account_valid(const pd_account_access *a);
bool pd_account_command(const char *line,size_t size,pd_account_request *out);
bool pd_account_token(const char *json,size_t size,const char *expected,int64_t now,char **out);
bool pd_account_catalog_parse(const char *json,size_t size,pd_account_catalog *out);
