
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_fs.h>
#include "dfs_romfs.h"

#include "luat_base.h"
#include "luat_fs.h"

#ifdef LUAT_USE_FS_VFS
extern const struct luat_vfs_filesystem vfs_fs_posix;
#ifdef LUAT_USE_VFS_INLINE_LIB
extern const char luadb_inline_sys[];
extern const struct luat_vfs_filesystem vfs_fs_luadb;
#endif
#endif

static void luatos(void* param) {
    rt_thread_mdelay(100); // 故意延后100ms
    luat_main();
    while (1)
        rt_thread_delay(10000000);
}

int rtt_luatos_init(void)
{
#ifdef LUAT_USE_FS_VFS
    // vfs进行必要的初始化
    luat_vfs_init(NULL);
    // 先注册luadb
	luat_vfs_reg(&vfs_fs_luadb);
    // 默认指向内部luadb
	luat_fs_conf_t conf2 = {
		.busname = (char*)luadb_inline_sys,
		.type = "luadb",
		.filesystem = "luadb",
		.mount_point = "/luadb/",
	};
    luat_fs_mount(&conf2);
#endif
    // rt_thread_t t = rt_thread_create("luatos", luatos, RT_NULL, 8*1024, 15, 20);
    // rt_thread_startup(t);
    return 0;
}
INIT_APP_EXPORT(rtt_luatos_init);

