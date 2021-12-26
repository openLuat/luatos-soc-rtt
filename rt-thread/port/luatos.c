
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_fs.h>
#include "dfs_romfs.h"

#include "luat_base.h"
#include "luat_fs.h"

extern const char luadb_inline_sys[];

extern const struct romfs_dirent luatos_romfs_root;

static void luatos(void* param) {
    rt_thread_mdelay(100); // 故意延后100ms
    luat_main();
    while (1)
        rt_thread_delay(10000000);
}

int rtt_luatos_init(void)
{
    if (dfs_mount(RT_NULL, "/", "rom", 0, &(luatos_romfs_root)) == 0){
        rt_kprintf("ROM file system initializated!\n");
    }else{
        rt_kprintf("ROM file system initializate failed!\n");
    }
    dfs_mount(RT_NULL, "/luadb", "luadb", 0, (const void *)luadb_inline_sys);

    rt_thread_t t = rt_thread_create("luatos", luatos, RT_NULL, 8*1024, 15, 20);
    rt_thread_startup(t);
    return 0;
}
INIT_APP_EXPORT(rtt_luatos_init);

