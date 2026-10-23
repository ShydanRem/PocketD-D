#include "diagnostics.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv){
    pd_diagnostics d={.firmware="0.54.0-test",.idf="v5.5.5",.uptime_ms=UINT64_MAX,
        .reset_reason=UINT32_MAX,.internal_free=UINT32_MAX,.internal_min=UINT32_MAX,
        .internal_largest=UINT32_MAX,.psram_total=UINT32_MAX,.psram_free=UINT32_MAX,
        .psram_min=UINT32_MAX,.psram_largest=UINT32_MAX,.ui_stack_min=UINT32_MAX,
        .usb_stack_min=UINT32_MAX,.lvgl_ticks=UINT32_MAX,.lvgl_max_us=UINT32_MAX,
        .flushes_submitted=UINT32_MAX,.touch_samples=UINT32_MAX,.touch_errors=UINT32_MAX,
        .touch_contacts=UINT32_MAX,.pwr_read_errors=UINT32_MAX,.pwr_presses=UINT32_MAX,
        .boot_presses=UINT32_MAX,.pwr_available=true,.state_writable=true,
        .history_available=true,.history_pending=true,.history_matches=true,
        .state_valid=true,.dimmed=true,.screen_failed=true,.busy=true};
    char out[PD_DIAGNOSTICS_MAX+1],small[PD_DIAGNOSTICS_MAX+2];
    size_t n=pd_diagnostics_encode(&d,out,sizeof out);assert(n&&n<=PD_DIAGNOSTICS_MAX);
    assert(strlen(out)==n&&!strchr(out,'\n')&&strstr(out,"18446744073709551615"));
    for(size_t size=0;size<=n;size++){
        memset(small,'X',sizeof small);assert(!pd_diagnostics_encode(&d,small,size));
        assert(small[size]=='X');if(size)assert(small[0]==0);
    }
    assert(pd_diagnostics_encode(&d,small,n+1)==n&&!strcmp(small,out));
    assert(!pd_diagnostics_encode(NULL,out,sizeof out)&&!out[0]);
    assert(!pd_diagnostics_encode(&d,NULL,sizeof out));
    const char *invalid[]={NULL,"","bad\"version","bad\\version","bad\nversion","bad version","123456789012345678901234567890123456789012345678901"};
    for(unsigned i=0;i<sizeof invalid/sizeof invalid[0];i++){
        d.firmware=invalid[i];assert(!pd_diagnostics_encode(&d,out,sizeof out)&&!out[0]);
        d.firmware="0.54.0";d.idf=invalid[i];assert(!pd_diagnostics_encode(&d,out,sizeof out));d.idf="v5.5.5";
    }
    memset(&d,0,sizeof d);d.firmware="0.54.0-test";d.idf="v5.5.5";
    n=pd_diagnostics_encode(&d,out,sizeof out);assert(n&&strstr(out,"\"state_writable\":false"));
    if(argc>1){FILE *f=fopen(argv[1],"wb");assert(f&&fwrite(out,1,n,f)==n);fclose(f);}
    puts("Diagnostics codec: bounded JSON, extreme counters, exact buffer sizes and unsafe version rejection passed (synthetic)");
}
