#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "class_looks.h"
#include "pocketdd.h"
#define PD_ASSET_BYTES (32u+PD_SPECIES_COUNT*24u*(4u+PD_LOOK_BYTES))
#define PD_ASSET_CHUNK 2048u
enum {PD_ASSET_HELLO,PD_ASSET_BEGIN,PD_ASSET_WRITE,PD_ASSET_FINISH,PD_ASSET_ABORT};
typedef struct {unsigned kind,size,offset,count;uint32_t crc;char sha[65];uint8_t data[PD_ASSET_CHUNK];} pd_asset_request;
bool pd_asset_parse(const char *line,size_t size,pd_asset_request *out);
/* Fixed application-owned filenames only; never accepts a path from USB.
 * verify checks the whole PDCL and optionally its SHA256; space observes SD.
 * All calls belong to the main/UI task. No NVS or character writes. */
typedef struct {FILE *file;char main[192],part[192],backup[192],directory[160],sha[65];unsigned offset;bool (*verify)(const char *,const char *);bool (*space)(uint64_t *);const char *error;} pd_asset_writer;
void pd_asset_writer_init(pd_asset_writer *writer,const char *directory,bool (*verify)(const char *,const char *),bool (*space)(uint64_t *));
/* 0:error; 1:ready for chunks; 2:exact package already verified on SD. */
unsigned pd_asset_begin(pd_asset_writer *writer,unsigned size,const char *sha);
bool pd_asset_write(pd_asset_writer *writer,const pd_asset_request *request);
bool pd_asset_finish(pd_asset_writer *writer,unsigned size);
void pd_asset_abort(pd_asset_writer *writer);
