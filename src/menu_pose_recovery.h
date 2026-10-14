/* Opt-in workaround, NOT an animation-memory patch. A verified native stance
 * frame change across UI can leave a bad foot pose. Use Skate's normal brake
 * action once, only on idle grounded input. No stick/stance remapping here. */
#ifndef SC_MENU_POSE_RECOVERY_H
#define SC_MENU_POSE_RECOVERY_H
typedef struct {
    unsigned long long selector;
    double deadline, until;
    int entered, entry_flag, pending, pulsing;
} sc_menu_pose_recovery;
static void sc_menu_pose_enter(sc_menu_pose_recovery* p,
                              unsigned long long selector,int state,int flag,int enabled) {
    sc_menu_pose_recovery zero={0};*p=zero;
    if(enabled && selector && state==100 && (flag==0 || flag==1)) {
        p->entered=1;p->selector=selector;p->entry_flag=flag;
    }
}
static int sc_menu_pose_exit(sc_menu_pose_recovery* p,
                            unsigned long long selector,int state,int flag,double now) {
    int changed=p->entered && selector==p->selector && state==100 &&
                (flag==0 || flag==1) && flag!=p->entry_flag;
    p->entered=0;p->pending=changed;p->pulsing=0;
    if(changed)p->deadline=now+1500;
    return changed;
}
static int sc_menu_pose_step(sc_menu_pose_recovery* p,unsigned long long selector,
                            int state,int enabled,int menu,double now,const sc_pad* raw) {
    if(!p->pending && !p->pulsing)return 0;
    if(!enabled || menu || selector!=p->selector || state!=100) {
        p->pending=p->pulsing=0;return 0;
    }
    /* Never combine a generated brake with a physical action or held stick.
     * Input arriving during a pulse ends it immediately. */
    int idle=!raw->buttons && raw->lt<=16 && raw->rt<=16 &&
             raw->lx>=-6000 && raw->lx<=6000 && raw->ly>=-6000 && raw->ly<=6000 &&
             raw->rx>=-6000 && raw->rx<=6000 && raw->ry>=-6000 && raw->ry<=6000;
    if(p->pulsing) {
        if(!idle || now>=p->until){p->pulsing=0;return 0;}
        return 1;
    }
    if(now>=p->deadline){p->pending=0;return 0;}
    if(!idle)return 0;
    p->pending=0;p->pulsing=1;p->until=now+120;
    return 1;
}
#endif
