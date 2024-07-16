#include "pal_internal.h"
#include "kernel_time.h"

struct pal_record {
    bool active;
    uint64_t last_time;
    uint64_t total;
    int count;
};

const struct pal_record DEFAULT_RECORD = {
    .active = false,
    .last_time = 0,
    .total = 0,
    .count = 0,
};

static struct pal_record in_kernel = DEFAULT_RECORD;
static struct pal_record in_syscall = DEFAULT_RECORD;
static struct pal_record in_proxy = DEFAULT_RECORD;
static struct pal_record in_user = DEFAULT_RECORD;

static uint64_t get_time_us(void) {
    uint64_t us;
    if (get_time_in_us(&us) < 0) {
        log_error("Failed to get time in us");
        return 0;
    }
    return us;
}

static void active_record(struct pal_record* record, bool active) {
    if (record->active == active) {
        return;
    }
    record->active = active;
    if (active) {
        record->last_time = get_time_us();
        record->count += 1;
    } else {
        uint64_t now = get_time_us();
        if (now < record->last_time) {
            log_error("Time went backwards, resetting last_time");
            return;
        }
        record->total += now - record->last_time;
        record->last_time = 0;
    }
}

void pal_active_record(int type, bool active) {
    switch (type) {
        case PAL_RECORD_KERNEL:
            active_record(&in_kernel, active);
            break;
        case PAL_RECORD_SYSCALL:
            active_record(&in_syscall, active);
            break;
        case PAL_RECORD_PROXY:
            active_record(&in_proxy, active);
            break;
        case PAL_RECORD_USER:
            active_record(&in_user, active);
            break;
        default:
            log_error("Unknown record type: %d", type);
    }
}

void pal_dump_and_reset_records(void) {
    log_always("PAL Records:");
    log_always("Kernel: %d calls, total time: %lu us", in_kernel.count, in_kernel.total);
    log_always("Syscall: %d calls, total time: %lu us", in_syscall.count, in_syscall.total);
    log_always("Proxy: %d calls, total time: %lu us", in_proxy.count, in_proxy.total);
    log_always("User: %d calls, total time: %lu us", in_user.count, in_user.total);

    in_kernel = DEFAULT_RECORD;
    in_syscall = DEFAULT_RECORD;
    in_proxy = DEFAULT_RECORD;
    in_user = DEFAULT_RECORD;
}
