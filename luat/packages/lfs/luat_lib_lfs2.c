

#include "luat_base.h"
#include "luat_sfd.h"
#include "luat_spi.h"
#include "luat_gpio.h"
#include "luat_malloc.h"
#include "luat_fs.h"

#define LUAT_LOG_TAG "lfs2"
#include "luat_log.h"

#include "lfs.h"

#ifdef LUAT_USE_FS_VFS
extern const struct luat_vfs_filesystem vfs_fs_lfs2;
#endif

int lfs_sfd_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_sfd_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_sfd_erase(const struct lfs_config *c, lfs_block_t block);
int lfs_sfd_sync(const struct lfs_config *c);

typedef struct lfs2_mount {
    char path[16];
    void* userdata;
    int luaref;
    lfs_t* fs;
    struct lfs_config* cfg;
}lfs2_mount_t;

static lfs2_mount_t mounted[2] = {0};

/**
挂载lifftefs,当前支持spi flash 和 memory 两种
@api lfs2.mount(path, drv, formatOnFail)
@string 挂载路径
@userdata 挂载所需要的额外数据, 当前仅支持sfd
@bool 挂载失败时是否尝试格式化,默认为false
@usage
local drv = sfd.init(0, 18)
if drv then
  local fs = lfs2.mount("/sfd", drv, true)
end
*/
static int l_lfs2_mount(lua_State *L) {
    const char* path = luaL_checkstring(L, 1);
    sfd_drv_t *drv = lua_touserdata(L, 2);
    bool formatOnFail = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false;
    for (size_t i = 0; i < 2; i++)
    {
        if (mounted[i].userdata == NULL) {
            mounted[i].userdata = drv;
            memcpy(mounted[i].path, path, strlen(path) + 1);
            
            lua_settop(L, 2);
            mounted[i].luaref = luaL_ref(L, LUA_REGISTRYINDEX);
            
            mounted[i].fs = luat_heap_malloc(sizeof(lfs_t));
            struct lfs_config* cfg = (struct lfs_config*)luat_heap_malloc(sizeof(struct lfs_config));
            mounted[i].cfg = cfg;

            memset(cfg, 0, sizeof(struct lfs_config));
            memset(mounted[i].fs, 0, sizeof(lfs_t));

            cfg->read  = lfs_sfd_read;
            cfg->prog  = lfs_sfd_prog;
            cfg->erase = lfs_sfd_erase;
            cfg->sync  = lfs_sfd_sync;

            cfg->read_size = 256;
            cfg->prog_size = 256;
            cfg->block_size = 4096;
            cfg->block_count = drv->sector_count / 16;
            cfg->block_cycles = 200;
            cfg->cache_size = 256;
            cfg->lookahead_size = 16;

            cfg->read_buffer = luat_heap_malloc(256);
            cfg->prog_buffer = luat_heap_malloc(256);
            cfg->lookahead_buffer = luat_heap_malloc(256);
            cfg->name_max = 63;
            cfg->file_max = 0;
            cfg->attr_max = 0;

            cfg->context = drv;

            int ret = lfs_mount(mounted[i].fs, cfg);
            LLOGW("lfs_mount ret %d", ret);
            if (ret < 0 && formatOnFail) {
              if (lfs_format(mounted[i].fs, cfg) == 0) {
                ret = lfs_mount(mounted[i].fs, cfg);
              }
            }
            if (ret < 0) {
                luat_heap_free(cfg->lookahead_buffer);
                luat_heap_free(cfg->prog_buffer);
                luat_heap_free(cfg->read_buffer);
                luat_heap_free(cfg);
                luat_heap_free(mounted[i].fs);
                mounted[i].fs = NULL;
                mounted[i].cfg = NULL;
                mounted[i].userdata = NULL;
            }
            else {
              #ifdef LUAT_USE_FS_VFS
              //char mount_point[16] = {0};
              //memcpy(mount_point, "/lfs2", strlen("/lfs2"));
              //memcpy(mount_point + strlen("/lfs2"), path, strlen(path));
              luat_fs_conf_t conf2 = {
		            .busname = (char*)mounted[i].fs,
		            .type = "lfs2",
		            .filesystem = "lfs2",
		            .mount_point = path,
	            };
	            luat_fs_mount(&conf2);
              #endif
            }
            lua_pushboolean(L, ret >= 0 ? 1 : 0);
            return 1;
        }
    }
    LLOGW("only 2 mount is allow");
    return 0;
}

/**
格式化为lifftefs
@api lfs2.mount(path)
@string 挂载路径
@usage
local drv = sfd.init("spi", 0, 18)
if drv then
  local fs = lfs2.mount("sfd", "/sfd", drv)
  if fs then
    lfs2.mkfs(fs)
  end
end
*/
static int l_lfs2_mkfs(lua_State *L) {
  const char* path = luaL_checkstring(L, 1);
  for (size_t i = 0; i < 2; i++) {
    if (mounted[i].userdata == NULL)
      continue;
    if (!strcmp(mounted[i].path, path)) {
      int ret = lfs_format(mounted[i].fs, mounted[i].cfg);
      LLOGD("lfs_format ret %d", ret);
      lua_pushboolean(L, ret == 0 ? 1 : 0);
      return 1;
    }
  }
  LLOGW("not path match, ignore mkfs");
  return 0;
}


#include "rotable.h"
static const rotable_Reg reg_lfs2[] =
{ 
  { "mount",	l_lfs2_mount, 0}, //初始化,挂载
//   { "unmount",	l_lfs2_unmount, 0}, // 取消挂载
  { "mkfs",		l_lfs2_mkfs, 0}, // 格式化!!!
//   //{ "test",  l_lfs2_test, 0},
//   { "getfree",	l_lfs2_getfree, 0}, // 获取文件系统大小,剩余空间
//   { "debug",	l_lfs2_debug_mode, 0}, // 调试模式,打印更多日志

//   { "lsdir",	l_lfs2_lsdir, 0}, // 列举目录下的文件,名称,大小,日期,属性
//   { "mkdir",	l_lfs2_mkdir, 0}, // 列举目录下的文件,名称,大小,日期,属性

//   { "stat",		l_lfs2_stat, 0}, // 查询文件信息
//   { "open",		l_lfs2_open, 0}, // 打开一个文件句柄
//   { "close",	l_lfs2_close, 0}, // 关闭一个文件句柄
//   { "seek",		l_lfs2_seek, 0}, // 移动句柄的当前位置
//   { "truncate",	l_lfs2_truncate, 0}, // 缩减文件尺寸到当前seek位置
//   { "read",		l_lfs2_read, 0}, // 读取数据
//   { "write",	l_lfs2_write, 0}, // 写入数据
//   { "remove",	l_lfs2_remove, 0}, // 删除文件,别名方法
//   { "unlink",	l_lfs2_remove, 0}, // 删除文件
//   { "rename",	l_lfs2_rename, 0}, // 文件改名

//   { "readfile",	l_lfs2_readfile, 0}, // 读取文件的简易方法
//   { "playmp3",	l_lfs2_playmp3, 0}, // 读取文件的简易方法
  { NULL,		NULL,	0 }
};

int luaopen_lfs2( lua_State *L )
{
  luat_newlib(L, reg_lfs2);
  #ifdef LUAT_USE_FS_VFS
  luat_vfs_reg(&vfs_fs_lfs2);
  #endif
  return 1;
}
