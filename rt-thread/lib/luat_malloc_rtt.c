
#include "luat_base.h"
#include "luat_malloc.h"
#include "rtthread.h"

#include "bget.h"

#ifdef BSP_USING_WM_LIBRARIES
    #include "wm_ram_config.h"
    #define W600_HEAP_ADDR 0x20028000
    #ifdef RT_USING_WIFI

    #else
    #define W600_MUC_HEAP_SIZE 64
    ALIGN(RT_ALIGN_SIZE) static char w600_mcu_heap[W600_MUC_HEAP_SIZE*1024]; // MCU模式下, rtt起码剩余140kb内存, 用64kb不过分吧

    #endif
#else
    #ifndef PKG_LUATOS_SOC_LUAT_HEAP_SIZE
        #ifdef SOC_FAMILY_STM32
            #define PKG_LUATOS_SOC_LUAT_HEAP_SIZE 64
        #else
            #define PKG_LUATOS_SOC_LUAT_HEAP_SIZE 128
        #endif
    #endif
    ALIGN(RT_ALIGN_SIZE) static char luavm_buff[PKG_LUATOS_SOC_LUAT_HEAP_SIZE*1024] = {0};
#endif

static int rtt_mem_init() {
    #ifdef BSP_USING_WM_LIBRARIES
    #ifdef RT_USING_WIFI
        // nothing
    #else
        // MUC heap , 占用一部分rtt heap
        bpool((void*)w600_mcu_heap, W600_MUC_HEAP_SIZE*1024);
        // 把wifi的内存全部吃掉
        bpool((void*)WIFI_MEM_START_ADDR, (64 - 16)*1024);
    #endif
    void *ptr = W600_HEAP_ADDR;
    #else
    char *ptr = (char*)luavm_buff;
    #endif
	bpool(ptr, PKG_LUATOS_SOC_LUAT_HEAP_SIZE*1024);
    return 0;
}
INIT_COMPONENT_EXPORT(rtt_mem_init);

void luat_meminfo_sys(size_t* total, size_t* used, size_t* max_used) {
    rt_memory_info(total, used, max_used);
}
