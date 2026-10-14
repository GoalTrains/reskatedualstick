/* Read the existing DelMar_UI_ScreenData model using the supported native
 * registry ABI. No UI writes, hooks or allocations. Caller holds model lock. */
typedef int (*sc_ui_read)(UINT64,void*,UINT64);
typedef UINT64 (*sc_ui_value)(UINT64,UINT64,UINT64,UINT64);
typedef struct { UINT64 manager,record; unsigned short type; } sc_ui_cache;
static UINT64 sc_ui_ptr(sc_ui_read read,UINT64 at){UINT64 p=0;return read(at,&p,8)?p:0;}
static int sc_ui_type_is_screen(sc_ui_read read,UINT64 table,unsigned short index){
    UINT64 entries=sc_ui_ptr(read,table+(UINT64)index*72+8);
    UINT64 type=entries?sc_ui_ptr(read,entries):0,meta=type?sc_ui_ptr(read,type):0;
    DWORD hash=0;unsigned short kind=0,size=0;
    return meta && read(meta,&hash,4) && hash==0x2c5645a1 &&
        read(meta+4,&kind,2) && ((kind>>5)&31)==2 && read(meta+6,&size,2) && size>=81 && size<=4096;
}
static int sc_ui_menu_read(UINT64 manager,sc_ui_read read,sc_ui_value value,sc_ui_cache* cache){
    struct Record {UINT64 id,value,root;unsigned short type,reserved;DWORD parent,index,padding;} record;
    UINT64 first=sc_ui_ptr(read,manager+0x98),last=sc_ui_ptr(read,manager+0xa0);
    UINT64 table=sc_ui_ptr(read,manager+0x600),end=sc_ui_ptr(read,manager+0x608);
    if(!manager || !first || last<first || (last-first)%40 || last-first>131072ull*40 ||
       !table || end<table || (end-table)%72 || end-table>65536ull*72){cache->record=0;return -1;}
    if(cache->manager!=manager){cache->manager=manager;cache->record=0;}
    if(cache->record && (cache->record<first || cache->record>=last ||
       !read(cache->record,&record,sizeof record) || record.parent!=0xffffffffu ||
       record.type!=cache->type || record.type>=(end-table)/72 || !sc_ui_type_is_screen(read,table,record.type)))cache->record=0;
    if(!cache->record)for(UINT64 at=first;at<last;at+=40){
        if(!read(at,&record,sizeof record))return -1;
        if(record.parent!=0xffffffffu || !record.root || !record.value || record.type>=(end-table)/72)continue;
        if(sc_ui_type_is_screen(read,table,record.type)){cache->record=at;cache->type=record.type;break;}
    }
    if(!cache->record)return -1;
    /* Typed storage; engine validates the root handle's generation. */
    UINT64 data=value(manager,record.root,0,1);BYTE flag=0;
    if(!data || !read(data+20,&flag,1) || flag>1){cache->record=0;return -1;}
    return flag;
}
