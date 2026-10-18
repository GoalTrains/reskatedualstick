#ifndef SC_GRIND_ENTRY_DIAG_H
#define SC_GRIND_ENTRY_DIAG_H
typedef struct {
 UINT64 qpc_us,object,context;
 DWORD valid,hooked,bad,calls,applied,consumed,requested,accepted;
 float yaw,pitch;
 /* Read-only pre-selector inputs to 47f7b90, not guessed trick labels.
  * values: aec, indexed 9a0, indexed 9b4, row+8, row+0,
  * row+4, indexed 9c8, constant 607acc8. Physical units unverified.
  * validity bit 0=row, 1=indexed values, 2=constant. */
 DWORD candidate_valid;int candidate_index,original_type,selected_type;
 float candidate_values[8];
 DWORD cache_valid_before,cache_valid_used;
 float cache_before,cache_used,cache_after;
 DWORD cache_valid_after;
} gd_native_entry;
#endif
