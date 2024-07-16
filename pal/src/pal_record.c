#include "pal.h"
#include "pal_internal.h"

void PalActiveRecord(int type, bool active) {
    pal_active_record(type, active);
}

void PalDumpAndResetRecords(void) {
    pal_dump_and_reset_records();
}