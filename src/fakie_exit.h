/* Exit through the game's named fakie timer. Never write stance, pose or transform flags. */
#ifndef SC_FAKIE_EXIT_H
#define SC_FAKIE_EXIT_H
#include "native_menu_respawn.h"
#include "native_pose_property.h"
typedef struct {nr_frame frame;np_address provider,graph,model,node,page,value;unsigned key;float timer;} fke_sample;
typedef struct {np_address owner;int previous,neutral,sent,writes;double neutral_since,landing_until,boost_until;} fke_state;
static int fke_name(np_address address,const char* expected,np_read_fn read){
 unsigned n=0;while(n<95&&expected[n])n++;if(n>=95)return 0;
 char actual[96];if(!read(address,actual,n+1))return 0;
 for(unsigned i=0;i<=n;i++)if(actual[i]!=expected[i])return 0;return 1;
}
static int fke_read(np_address selector,np_address base,np_read_fn read,fke_sample* out){
 np_address bool_node,bool_page,bool_value,vt;unsigned bool_key,bits;unsigned char riding;
 if(!nr_read(selector,base,read,&out->frame)||
    !np_pointer(read,out->frame.core+0x630,&out->provider)||!np_pointer(read,out->frame.core+0x610,&out->model)||
    !read(out->provider+0x38,&out->graph,8))return 0;
 out->graph&=~4ULL;
 if(out->graph<0x10000||(out->graph&7)||
    !npp_value(out->graph,0x170,out->model,base,read,&bool_node,&bool_page,&bool_value,&bool_key,&riding)||
    !fke_name(bool_node+0x88,"Animation/Dingo/Bool.Physics.RidingFakie",read)||riding!=out->frame.native_frame||
    !read(out->graph+0x188,&out->node,8))return 0;
 out->node&=~4ULL;
 if(out->node<0x10000||(out->node&7)||!read(out->node+0x20,&vt,8)||vt!=base+0x61ab550||
    !fke_name(out->node+0x78,"Animation/Dingo/Float.Physics.TimeRidingFakie",read)||
    !read(out->node+0x28,&out->key,4)||(out->key>>24)!=4)return 0;
 unsigned index=(out->key>>12)&0x7ff;
 if((index&3)||index>0x718||!np_pointer(read,out->model+0x60+(np_address)(out->key&0xfff)*8,&out->page)||
    !read(out->page+0x71c+(index>>5)*4,&bits,4)||!(bits&(1u<<(index&31))))return 0;
 out->value=out->page+index;
 return read(out->value,&out->timer,4)&&__builtin_isfinite(out->timer)&&out->timer>=0&&out->timer<=60;
}
static int fke_step(fke_state* s,int enabled,int mode,int state,int idle,int protect,double now,np_address selector,np_address base,np_read_fn read,np_write_fn write){
 if(!enabled||!selector||!base){fke_state empty={0};*s=empty;return 0;}
 if(s->owner!=selector){fke_state empty={0};*s=empty;s->owner=selector;}
 int previous=s->previous;s->previous=state;
 if(((previous>=200&&previous<300)||(previous>=400&&previous<500))&&state>=100&&state<200)s->landing_until=now+1200;
 /* A requested manual, held trick setup or handplant outranks optional stance normalization. */
 if(protect){s->neutral=0;s->boost_until=0;return 0;}
 int automatic=mode==2&&s->landing_until>now;
 if(state!=100&&!(automatic&&state==103)){s->neutral=0;s->boost_until=0;return 0;}
 fke_sample a;if(!fke_read(selector,base,read,&a)){s->neutral=0;s->boost_until=0;return 0;}
 if(!a.frame.native_frame){s->neutral=s->sent=s->writes=0;s->boost_until=0;return 0;}
 if(!idle&&!automatic){s->neutral=0;s->boost_until=0;return 0;}
 if(!s->neutral){s->neutral=1;s->neutral_since=now;}
 if(!automatic&&now-s->neutral_since<20)return 0;
 if(!s->sent){s->sent=1;s->writes=0;s->boost_until=now+120;}
 if(now>s->boost_until||s->writes>=3||a.timer<=0||a.timer>=1.6f)return 0;
 fke_sample b;
 if(!fke_read(selector,base,read,&b)||!nr_same(&a.frame,&b.frame)||!b.frame.native_frame||
    a.provider!=b.provider||a.graph!=b.graph||a.model!=b.model||a.node!=b.node||a.page!=b.page||a.value!=b.value||a.key!=b.key||b.timer<=0||b.timer>=1.6f)return 0;
 float target=1.6f;if(!write(b.value,&target,4))return -1;
 s->writes++;return s->writes==1?1:2;
}
#endif
