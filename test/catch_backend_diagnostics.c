/* Reuse the real Windows reader fixture and declarations, but exercise the
 * separate catch recording policy without starting its asynchronous writer. */
#define main pose_fixture_main
#include "pose_diagnostics.c"
#undef main
int main(void) {
 BYTE* mem=VirtualAlloc(0,131072,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(mem);
 *(UINT64*)mem=(UINT64)(mem+4096);
 *(UINT64*)(mem+16)=(UINT64)(mem+16384);
 *(UINT64*)(mem+16384+16)=(UINT64)(mem+18432);
 *(UINT64*)(mem+18432+0x630)=(UINT64)(mem+65536);
 *(UINT64*)(mem+65536+0x28)=(UINT64)(mem+69632)|4;
 *(UINT64*)(mem+18432+0x610)=(UINT64)(mem+73728);
 const unsigned offsets[4]={0x1e8,0x1f0,0x278,0x408};
 for(int i=0;i<4;i++){
  *(UINT64*)(mem+69632+offsets[i])=(UINT64)(mem+81920+i*512)|4;
  mem[81920+i*512]=42+i;
  *(DWORD*)(mem+81920+i*512+0x28)=0x01000001u+(i<<12);
 }
 *(UINT64*)(mem+73728+0x48+0x18+8)=(UINT64)(mem+90112);
 BYTE* before=VirtualAlloc(0,131072,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(before);
 memcpy(before,mem,131072);
 int probes[4]={0};sc_pad raw={0},out={0};
 pd_catch_mode=1;pd_catch_meta=0x020302;pd_started=1;
 pd_capture(1000,(UINT64)mem,100,0,1,0,probes,&raw,&out);assert(pd_samples==1);
 for(int i=0;i<100;i++)pd_capture(1100+i*100,(UINT64)mem,100,0,1,0,probes,&raw,&out);
 assert(pd_samples==1); /* Idle cannot exhaust the budget. */
 pd_capture(12000,(UINT64)mem,103,0,1,0,probes,&raw,&out);assert(pd_samples==2);
 for(int i=0;i<4;i++)assert(pd_queue[1].model_nodes[i][0]==42+i);
 assert(pd_queue[1].reserved==0x020302 && pd_queue[1].snapshot.label==1);
 assert(pd_queue[1].property_valid==15);
 raw.buttons=0x40;
 pd_capture(12001,(UINT64)mem,200,0,1,0,probes,&raw,&out);
 assert(pd_samples==3 && (pd_queue[2].events&32)); /* Fresh catch, no 10 ms delay. */
 raw.buttons=0;
 pd_capture(12002,(UINT64)mem,200,0,1,0,probes,&raw,&out);
 assert(pd_samples==4 && (pd_queue[3].events&32));
 pd_capture(12150,(UINT64)mem,100,0,1,0,probes,&raw,&out);assert(pd_samples==5);
 pd_capture(12600,(UINT64)mem,100,0,1,0,probes,&raw,&out);assert(pd_samples==5);
 assert(!memcmp(before,mem,131072));assert(!raw.buttons && !out.buttons);
 for(unsigned i=0;i<PD_CATCH_LIMIT+10;i++){
  pd_read=pd_write;
  pd_capture(13000+i*100,(UINT64)mem,200,0,1,0,probes,&raw,&out);
 }
 assert(pd_samples==PD_CATCH_LIMIT && pd_limit_logged);
 assert(16+sizeof(pd_record)*pd_samples==11350032);
 /* The actual worker uses a separate file and identifies the catch schema. */
 pd_read=0;pd_write=1;
 DWORD id;HANDLE thread=CreateThread(0,0,pd_worker,0,0,&id);assert(thread);CloseHandle(thread);Sleep(150);
 HANDLE file=CreateFileW(L"C:\\Games\\Skate\\mine\\reskatedualstick-source\\build\\session_controls_catch_backend.bin",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,OPEN_EXISTING,0,0);assert(file!=INVALID_HANDLE_VALUE);
 BYTE header[16];DWORD n;assert(ReadFile(file,header,16,&n,0)&&n==16);
 assert(!memcmp(header,"SCPOSE05",8) && *(DWORD*)(header+8)==sizeof(pd_record) && *(DWORD*)(header+12)==611);
 CloseHandle(file);
 puts("PASS: catch-specific nodes/pages, idle exclusion, immediate catch edges, landing tail, read-only packets/memory and 11.4 MB binary cap");
 return 0;
}
