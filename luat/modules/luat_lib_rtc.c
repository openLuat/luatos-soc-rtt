/*
@module  rtc
@summary 实时时钟
@version 1.0
@date    2021.08.31
@demo rtc
*/
#include "luat_base.h"
#include "luat_rtc.h"

#define LUAT_LOG_TAG "rtc"
#include "luat_log.h"

int Base_year = 1900;

void LUAT_WEAK luat_rtc_set_tamp32(uint32_t tamp) {
    LLOGD("not support yet");
}

/*
设置时钟
@api rtc.set(tab)
@table or int 时钟参数,见示例
@return bool 成功返回true,否则返回nil或false
@usage
rtc.set({year=2021,mon=8,day=31,hour=17,min=8,sec=43})
rtc.set(1652230554)	--目前只有105支持时间戳方式
*/
static int l_rtc_set(lua_State *L){
    struct tm tblock = {0};
    int ret;
    if (!lua_istable(L, 1)) {
    	if (lua_isinteger(L, 1))
    	{
    		uint32_t tamp = lua_tointeger(L, 1);
    	    luat_rtc_set_tamp32(tamp);
    	    lua_pushboolean(L, 1);
    	    return 1;
    	}
        LLOGW("rtc time need table");
        return 0;
    }
    lua_settop(L, 1);

    lua_pushstring(L, "year");
    lua_gettable(L, 1);
    if (lua_isnumber(L, -1)) {
        tblock.tm_year = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss year");
        return 0;
    }

    lua_pushstring(L, "mon");
    lua_gettable(L, 1);
    if (lua_isnumber(L, -1)) {
        tblock.tm_mon = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss mon");
        return 0;
    }

    lua_pushstring(L, "day");
    lua_gettable(L, 1);
    if (lua_isnumber(L, -1)) {
        tblock.tm_mday = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss day");
        return 0;
    }

    lua_pushstring(L, "hour");
    lua_gettable(L, 1);
    if (lua_isnumber(L, -1)) {
        tblock.tm_hour = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss hour");
        return 0;
    }

    lua_pushstring(L, "min");
    lua_gettable(L, 1);
    if (lua_isnumber(L, -1)) {
        tblock.tm_min = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss min");
        return 0;
    }

    lua_pushstring(L, "sec");
    lua_gettable(L, 1);
    if (lua_isnumber(L, -1)) {
        tblock.tm_sec = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss sec");
        return 0;
    }

    ret = luat_rtc_set(&tblock);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
获取时钟
@api rtc.get()
@return table 时钟参数,见示例
@usage
local t = rtc.get()
-- {year=2021,mon=8,day=31,hour=17,min=8,sec=43}
log.info("rtc", json.encode(t))
*/
static int l_rtc_get(lua_State *L){
    struct tm tblock = {0};
    int ret;
    ret = luat_rtc_get(&tblock);
    if (ret) {
        return 0;
    }
    lua_newtable(L);

    lua_pushstring(L, "year");
    lua_pushinteger(L, tblock.tm_year );
    lua_settable(L, -3);

    lua_pushstring(L, "mon");
    lua_pushinteger(L, tblock.tm_mon );
    lua_settable(L, -3);

    lua_pushstring(L, "day");
    lua_pushinteger(L, tblock.tm_mday);
    lua_settable(L, -3);

    lua_pushstring(L, "hour");
    lua_pushinteger(L, tblock.tm_hour);
    lua_settable(L, -3);

    lua_pushstring(L, "min");
    lua_pushinteger(L, tblock.tm_min);
    lua_settable(L, -3);

    lua_pushstring(L, "sec");
    lua_pushinteger(L, tblock.tm_sec);
    lua_settable(L, -3);

    return 1;
}

/*
设置RTC唤醒时间
@api rtc.timerStart(id, tab)
@int 时钟id,通常只支持0
@table 时钟参数,见示例
@return bool 成功返回true,否则返回nil或false
@usage
-- 使用前建议先rtc.set设置为正确的时间
rtc.timerStart(0, {year=2021,mon=9,day=1,hour=17,min=8,sec=43})
*/
static int l_rtc_timer_start(lua_State *L){
    int id;
    struct tm tblock = {0};
    int ret;

    id = luaL_checkinteger(L, 1);
    if (!lua_istable(L, 2)) {
        LLOGW("rtc time need table");
        return 0;
    }

    lua_settop(L, 2);
    lua_pushstring(L, "year");
    lua_gettable(L, 2);
    if (lua_isnumber(L, -1)) {
        tblock.tm_year = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss year");
        return 0;
    }

    lua_pushstring(L, "mon");
    lua_gettable(L, 2);
    if (lua_isnumber(L, -1)) {
        tblock.tm_mon = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss mon");
        return 0;
    }

    lua_pushstring(L, "day");
    lua_gettable(L, 2);
    if (lua_isnumber(L, -1)) {
        tblock.tm_mday = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss day");
        return 0;
    }

    lua_pushstring(L, "hour");
    lua_gettable(L, 2);
    if (lua_isnumber(L, -1)) {
        tblock.tm_hour = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss hour");
        return 0;
    }

    lua_pushstring(L, "min");
    lua_gettable(L, 2);
    if (lua_isnumber(L, -1)) {
        tblock.tm_min = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss min");
        return 0;
    }

    lua_pushstring(L, "sec");
    lua_gettable(L, 2);
    if (lua_isnumber(L, -1)) {
        tblock.tm_sec = luaL_checkinteger(L, -1);
    }
    else {
        LLOGW("rtc time miss sec");
        return 0;
    }

    ret = luat_rtc_timer_start(id, &tblock);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
取消RTC唤醒时间
@api rtc.timerStop(id)
@int 时钟id,通常只支持0
@return bool 成功返回true,否则返回nil或false
@usage
rtc.timerStop(0)
*/
static int l_rtc_timer_stop(lua_State *L){
    int id;
    int ret;

    id = luaL_checkinteger(L, 1);
    ret = luat_rtc_timer_stop(id);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
设置RTC基准年
@api rtc.setBaseYear(Base_year)
@int 基准年Base_year,通常1900
@usage
rtc.setBaseYear(1900)
*/
static int l_rtc_set_base_year(lua_State *L){
    Base_year = luaL_checkinteger(L, 1);
    return 0;
}


#include "rotable2.h"
static const rotable_Reg_t reg_rtc[] =
{
    { "set",        ROREG_FUNC(l_rtc_set)},
    { "get",        ROREG_FUNC(l_rtc_get)},
    { "timerStart", ROREG_FUNC(l_rtc_timer_start)},
    { "timerStop",  ROREG_FUNC(l_rtc_timer_stop)},
    { "setBaseYear", ROREG_FUNC(l_rtc_set_base_year)},
	{ NULL,         ROREG_INT(0) }
};

LUAMOD_API int luaopen_rtc( lua_State *L ) {
    luat_newlib2(L, reg_rtc);
    return 1;
}
