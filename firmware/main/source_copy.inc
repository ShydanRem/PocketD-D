/* Match only explicit Beyond provenance, never display names. Read-only stage. */
enum {IC_CANCEL=8350,IC_NEW,IC_BACK};
static unsigned source_copy_count,source_copy_slot;
static bool source_copy_approved;
static page_t source_copy_return;
static uint32_t source_copy_origin,source_copy_candidate;
static const char *source_copy_notice="";
static bool source_copy_open(void){return import_pending&&page==IMPORT_COMPARE;}
static bool source_copy_current(void){return source_copy_origin==pd_crc32((const uint8_t *)&state,sizeof state)&&source_copy_candidate==pd_crc32((const uint8_t *)&pending,sizeof pending);}
static bool source_copy_prepare(void){
    source_copy_count=source_copy_slot=0;source_copy_approved=false;source_copy_notice="";
    source_copy_origin=pd_crc32((const uint8_t *)&state,sizeof state);source_copy_candidate=pd_crc32((const uint8_t *)&pending,sizeof pending);
    if(!pending.beyond_id[0])return true;
    bool active=!strcmp(state.beyond_id,pending.beyond_id);unsigned slot=0;
    if(active){source_copy_count=1;hit_candidate=state;}
    if(pd_board_library_root())for(unsigned i=0;i<library_catalog.count;i++){
        const pd_library_entry *e=library_catalog.entries+i;
        if((e->status!=PD_LIBRARY_OK&&e->status!=PD_LIBRARY_RECOVERED)||strcmp(e->beyond_id,pending.beyond_id)||(active&&e->epoch==state.history_epoch&&e->owner==library.owner))continue;
        source_copy_count++;if(!active&&!slot)slot=e->slot;
    }
    source_copy_slot=slot;
    if(slot){pd_backup *copy=malloc(sizeof *copy);if(!copy)return false;pd_library_result r=pd_library_read(&library,slot,copy);bool ok=(r==PD_LIBRARY_OK||r==PD_LIBRARY_RECOVERED)&&!strcmp(copy->state.beyond_id,pending.beyond_id);if(ok)hit_candidate=copy->state;free(copy);if(!ok)return false;}
    return true;
}

static void source_copy_keep(void){
    if(!source_copy_slot){page=HERO;return;}
    char wanted[PD_PROVENANCE_BYTES];memcpy(wanted,pending.beyond_id,sizeof wanted);unsigned slot=source_copy_slot;
    library_open();
    for(unsigned i=0;i<library_catalog.count;i++)if(library_catalog.entries[i].slot==slot&&!strcmp(library_catalog.entries[i].beyond_id,wanted)){library_select(i);return;}
    snprintf(library_notice,sizeof library_notice,"Copia cambiata: scegli dalla raccolta");
}
