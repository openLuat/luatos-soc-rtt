
/*
@module  sdio
@summary sdio
@version 1.0
@date    2021.09.02
*/
#include "luat_base.h"
#include "luat_sdio.h"
#include "luat_malloc.h"

#define SDIO_COUNT 2
static luat_sdio_t sdio_t[SDIO_COUNT];

/**
初始化sdio
@api sdio.init(id)
@int 通道id,与具体设备有关,通常从0开始,默认0
@return boolean 打开结果
 */
static int l_sdio_init(lua_State *L) {
    if (luat_sdio_init(luaL_optinteger(L, 1, 0)) == 0) {
        lua_pushboolean(L, 1);
    }
    else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

/*
直接读写sd卡上的数据
@api sdio.sd_read(id, offset, len)
@int sdio总线id
@int 偏移量,必须是512的倍数
@int 长度,必须是512的倍数
@return string 若读取成功,返回字符串,否则返回nil
@usage
-- 初始化sdio并直接读取sd卡数据
sdio.init(0)
local t = sdio.sd_read(0, 0, 1024)
if t then
    --- xxx
end
*/
static int l_sdio_read(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    int offset = luaL_checkinteger(L, 2);
    int len = luaL_checkinteger(L, 3);
    char* recv_buff = luat_heap_malloc(len);
    if(recv_buff == NULL)
        return 0;
    int ret = luat_sdio_sd_read(id, sdio_t[id].rca, recv_buff, offset, len);
    if (ret > 0) {
        lua_pushlstring(L, recv_buff, ret);
        luat_heap_free(recv_buff);
        return 1;
    }
    luat_heap_free(recv_buff);
    return 0;
}

/*
直接写sd卡
@api sdio.sd_write(id, data, offset)
@int sdio总线id
@string 待写入的数据,长度必须是512的倍数
@int 偏移量,必须是512的倍数
@return bool 若读取成功,返回true,否则返回false
@usage
-- 初始化sdio并直接读取sd卡数据
sdio.init(0)
local t = sdio.sd_write(0, data, 0)
if t then
    --- xxx
end
*/
static int l_sdio_write(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    size_t len;
    const char* send_buff;
    send_buff = lua_tolstring(L, 2, &len);
    int offset = luaL_checkinteger(L, 3);
    int ret = luat_sdio_sd_write(id, sdio_t[id].rca, (char*)send_buff, offset, len);
    if (ret > 0) {
        lua_pushboolean(L, 1);
    }
    lua_pushboolean(L, 0);
    return 1;
}

/*
挂载SD卡, 使用FATFS文件系统
@api sdio.sd_mount(id, path, auto_format)
@int sdio总线id
@string 挂载路径, 默认"/sd", 不允许以"/"结尾
@bool 是否自动格式化,默认是true
@return bool 挂载成功返回true,否则返回false
@return int 底层返回的具体结果码,用于调试
*/
static int l_sdio_sd_mount(lua_State *L) {
    int id = luaL_optinteger(L, 1, 0);
    const char* path = luaL_optstring(L, 2, "/sd");
    int auto_format = luaL_optinteger(L, 3, 1);
    int ret = luat_sdio_sd_mount(id, &sdio_t[id].rca, (char*)path, auto_format);
    lua_pushboolean(L, ret == 0 ? 1: 0);
    lua_pushinteger(L, ret);
    return 2;
}

/*
卸载SD卡(视硬件情况, 不一定支持)
@api sdio.sd_unmount(id, path)
@int sdio总线id
@string 挂载路径, 默认"/sd", 不允许以"/"结尾
@return bool 挂载成功返回true,否则返回false
*/
static int l_sdio_sd_umount(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    //int auto_format = luaL_checkinteger(L, 2);
    int ret = luat_sdio_sd_unmount(id, sdio_t[id].rca);
    lua_pushboolean(L, ret == 0 ? 1: 0);
    return 1;
}

/*
格式化SD卡
@api sdio.sd_format(id)
@int sdio总线id
@return bool 挂载成功返回true,否则返回false
*/
static int l_sdio_sd_format(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    //int auto_format = luaL_checkinteger(L, 2);
    int ret = luat_sdio_sd_format(id, sdio_t[id].rca);
    lua_pushboolean(L, ret == 0 ? 1: 0);
    return 1;
}

#include "rotable.h"
static const rotable_Reg reg_sdio[] =
{
    { "init" ,       l_sdio_init , 0},
    { "sd_read" ,       l_sdio_read , 0},
    { "sd_write" ,      l_sdio_write, 0},
    { "sd_mount" ,      l_sdio_sd_mount, 0},
    { "sd_umount" ,     l_sdio_sd_umount, 0},
    { "sd_format" ,     l_sdio_sd_format, 0},
	{ NULL,          NULL ,       0}
};

LUAMOD_API int luaopen_sdio( lua_State *L ) {
    luat_newlib(L, reg_sdio);
    return 1;
}
