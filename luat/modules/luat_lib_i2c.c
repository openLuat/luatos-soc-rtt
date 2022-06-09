
/*
@module  i2c
@summary I2C操作
@version 1.0
@date    2020.03.30
@demo i2c
*/
#include "luat_base.h"
#include "luat_log.h"
#include "luat_timer.h"
#include "luat_malloc.h"
#include "luat_i2c.h"
#include "luat_gpio.h"
#include "luat_zbuff.h"
#define LUAT_LOG_TAG "i2c"
#include "luat_log.h"

static void i2c_soft_start(luat_ei2c *ei2c)
{
    luat_gpio_mode(ei2c->sda, Luat_GPIO_OUTPUT, Luat_GPIO_PULLUP, 1);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->scl, Luat_GPIO_HIGH);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->sda, Luat_GPIO_LOW);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->scl, Luat_GPIO_LOW);
    luat_timer_us_delay(5);
}
static void i2c_soft_stop(luat_ei2c *ei2c)
{
    luat_gpio_mode(ei2c->sda, Luat_GPIO_OUTPUT, Luat_GPIO_PULLUP, 1);
    luat_gpio_set(ei2c->scl, Luat_GPIO_LOW);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->sda, Luat_GPIO_LOW);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->scl, Luat_GPIO_HIGH);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->sda, Luat_GPIO_HIGH);
    luat_timer_us_delay(5);
}
static unsigned char i2c_soft_wait_ack(luat_ei2c *ei2c)
{
    luat_gpio_set(ei2c->sda, Luat_GPIO_HIGH);
    luat_gpio_mode(ei2c->sda, Luat_GPIO_INPUT, Luat_GPIO_PULLUP, 1);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->scl, Luat_GPIO_HIGH);
    luat_timer_us_delay(15);
    int max_wait_time = 3000;
    while (max_wait_time--)
    {
        if (luat_gpio_get(ei2c->sda) == Luat_GPIO_LOW)
        {
            luat_gpio_set(ei2c->scl, Luat_GPIO_LOW);
            return 1;
        }
        luat_timer_us_delay(1);
    }
    i2c_soft_stop(ei2c);
    return 0;
}
static void i2c_soft_send_ack(luat_ei2c *ei2c)
{
    luat_gpio_mode(ei2c->sda, Luat_GPIO_OUTPUT, Luat_GPIO_PULLUP, 0);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->scl, Luat_GPIO_HIGH);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->scl, Luat_GPIO_LOW);
}
static void i2c_soft_send_noack(luat_ei2c *ei2c)
{
    luat_gpio_mode(ei2c->sda, Luat_GPIO_OUTPUT, Luat_GPIO_PULLUP, 1);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->scl, Luat_GPIO_HIGH);
    luat_timer_us_delay(5);
    luat_gpio_set(ei2c->scl, Luat_GPIO_LOW);
}
static void i2c_soft_send_byte(luat_ei2c *ei2c, unsigned char data)
{
    unsigned char i = 8;
    luat_gpio_mode(ei2c->sda, Luat_GPIO_OUTPUT, Luat_GPIO_PULLUP, 0);
    while (i--)
    {
        luat_gpio_set(ei2c->scl, Luat_GPIO_LOW);
        luat_timer_us_delay(10);
        if (data & 0x80)
        {
            luat_gpio_set(ei2c->sda, Luat_GPIO_HIGH);
        }
        else
        {
            luat_gpio_set(ei2c->sda, Luat_GPIO_LOW);
        }
        luat_timer_us_delay(5);
        data <<= 1;
        luat_gpio_set(ei2c->scl, Luat_GPIO_HIGH);
        luat_timer_us_delay(5);
        luat_gpio_set(ei2c->scl, Luat_GPIO_LOW);
        luat_timer_us_delay(5);
    }
}
static char i2c_soft_recv_byte(luat_ei2c *ei2c)
{
    unsigned char i = 8;
    unsigned char data = 0;
    luat_gpio_set(ei2c->sda, Luat_GPIO_HIGH);
    luat_gpio_mode(ei2c->sda, Luat_GPIO_INPUT, Luat_GPIO_PULLUP, 1);
    while (i--)
    {
        data <<= 1;
        luat_gpio_set(ei2c->scl, Luat_GPIO_LOW);
        luat_timer_us_delay(5);
        luat_gpio_set(ei2c->scl, Luat_GPIO_HIGH);
        luat_timer_us_delay(5);
        if (luat_gpio_get(ei2c->sda))
            data |= 0x01;
    }
    luat_gpio_set(ei2c->scl, Luat_GPIO_LOW);
    return (data);
}
static char i2c_soft_recv(luat_ei2c *ei2c, unsigned char addr, char *buff, size_t len)
{
    size_t i;
    i2c_soft_start(ei2c);
    i2c_soft_send_byte(ei2c, (addr << 1) + 1);
    if (!i2c_soft_wait_ack(ei2c))
    {
        i2c_soft_stop(ei2c);
        return -1;
    }
    luat_timer_us_delay(50);
    for (i = 0; i < len; i++)
    {
        *buff++ = i2c_soft_recv_byte(ei2c);
        if (i < (len - 1))
            i2c_soft_send_ack(ei2c);
    }
    i2c_soft_send_noack(ei2c);
    i2c_soft_stop(ei2c);
    return 0;
}
static char i2c_soft_send(luat_ei2c *ei2c, unsigned char addr, char *data, size_t len, uint8_t stop)
{
    size_t i;
    i2c_soft_start(ei2c);
    i2c_soft_send_byte(ei2c, addr << 1);
    if (!i2c_soft_wait_ack(ei2c))
    {
        i2c_soft_stop(ei2c);
        return -1;
    }
    for (i = 0; i < len; i++)
    {
        i2c_soft_send_byte(ei2c, data[i]);
        if (!i2c_soft_wait_ack(ei2c))
        {
            i2c_soft_stop(ei2c);
            return -1;
        }
    }
    if (stop){
        i2c_soft_stop(ei2c);
    }
    return 0;
}

/*
i2c编号是否存在
@api i2c.exist(id)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@return int 存在就返回1,否则返回0
@usage
-- 检查i2c1是否存在
if i2c.exist(1) then
    log.info("存在 i2c1")
end
*/
static int l_i2c_exist(lua_State *L)
{
    int re = luat_i2c_exist(luaL_checkinteger(L, 1));
    lua_pushinteger(L, re);
    return 1;
}

/*
i2c初始化
@api i2c.setup(id, speed, slaveAddr)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int I2C速度, 例如i2c.FAST
@int 从设备地址（7位）, 例如0x38
@return int 成功就返回1,否则返回0
@usage
-- 初始化i2c1
if i2c.setup(1, i2c.FAST, 0x38) == 1 then
    log.info("存在 i2c1")
else
    i2c.close(1) -- 关掉
end
*/
static int l_i2c_setup(lua_State *L)
{
    int re = luat_i2c_setup(luaL_checkinteger(L, 1), luaL_optinteger(L, 2, 0), luaL_optinteger(L, 3, 0));
    lua_pushinteger(L, re == 0 ? luaL_optinteger(L, 2, 0) : -1);
    return 1;
}

/*
新建一个软件i2c对象
@api i2c.createSoft(scl,sda,slaveAddr)
@int i2c SCL引脚编号
@int i2c SDA引脚编号
@int 从设备地址（7位）, 例如0x38
@return 软件I2C对象 可当作i2c的id使用
@usage
-- 注意！这个接口是软件模拟i2c，速度可能会比硬件的慢
-- 不需要调用i2c.close接口
-- 初始化软件i2c
local softI2C = i2c.createSoft(1,2,0x38)
i2c.send(softI2C, 0x5C, string.char(0x0F, 0x2F))
*/
static int l_i2c_soft(lua_State *L)
{
    luat_ei2c *ei2c = (luat_ei2c *)lua_newuserdata(L, sizeof(luat_ei2c));
    ei2c->scl = luaL_checkinteger(L, 1);
    ei2c->sda = luaL_checkinteger(L, 2);
    ei2c->addr = luaL_checkinteger(L, 3);
    luat_gpio_mode(ei2c->scl, Luat_GPIO_OUTPUT, Luat_GPIO_PULLUP, 1);
    luat_gpio_mode(ei2c->sda, Luat_GPIO_OUTPUT, Luat_GPIO_PULLUP, 1);
    i2c_soft_stop(ei2c);
    luaL_setmetatable(L, LUAT_EI2C_TYPE);
    return 1;
}

/*
i2c发送数据
@api i2c.send(id, addr, data,stop)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int I2C子设备的地址, 7位地址
@integer/string/table 待发送的数据,自适应参数类型
@integer 可选参数 是否发送停止位 1发送 0不发送 默认发送(105不支持)
@return true/false 发送是否成功
@usage
-- 往i2c0发送1个字节的数据
i2c.send(0, 0x68, 0x75)
-- 往i2c1发送2个字节的数据
i2c.send(1, 0x5C, string.char(0x0F, 0x2F))
i2c.send(1, 0x5C, {0x0F, 0x2F})
*/
static int l_i2c_send(lua_State *L)
{
    int id = 0;
    if (!lua_isuserdata(L, 1))
    {
        id = luaL_checkinteger(L, 1);
    }
    int addr = luaL_checkinteger(L, 2);
    size_t len = 0;
    int result = 0;

    int stop = luaL_optnumber(L, 4 , 1);
    if (lua_isinteger(L, 3))
    {
        char buff = (char)luaL_checkinteger(L, 3);
        if (lua_isuserdata(L, 1))
        {
            luat_ei2c *ei2c = toei2c(L);
            result = i2c_soft_send(ei2c, addr, &buff, 1,stop);
        }
        else
        {
            result = luat_i2c_send(id, addr, &buff, 1,stop);
        }
        // luat_heap_free(buff);
    }
    else if (lua_isstring(L, 3))
    {
        const char *buff = luaL_checklstring(L, 3, &len);
        if (lua_isuserdata(L, 1))
        {
            luat_ei2c *ei2c = toei2c(L);
            result = i2c_soft_send(ei2c, addr, (char *)buff, len,stop);
        }
        else
        {
            result = luat_i2c_send(id, addr, (char *)buff, len,stop);
        }
    }
    else if (lua_istable(L, 3))
    {
        const int len = lua_rawlen(L, 3); //返回数组的长度
        char *buff = (char *)luat_heap_malloc(len);

        for (size_t i = 0; i < len; i++)
        {
            lua_rawgeti(L, 3, 1 + i);
            buff[i] = (char)lua_tointeger(L, -1);
            lua_pop(L, 1); //将刚刚获取的元素值从栈中弹出
        }
        if (lua_isuserdata(L, 1))
        {
            luat_ei2c *ei2c = toei2c(L);
            result = i2c_soft_send(ei2c, addr, buff, len,stop);
        }
        else
        {
            result = luat_i2c_send(id, addr, buff, len,stop);
        }
        luat_heap_free(buff);
    }
    else
    {
        if (lua_isuserdata(L, 1))
        {
            luat_ei2c *ei2c = toei2c(L);
            result = i2c_soft_send(ei2c, addr, NULL, 0,stop);
        }
        else
        {
            result = luat_i2c_send(id, addr, NULL, 0,stop);
        }
    }
    lua_pushboolean(L, result == 0);
    return 1;
}

/*
i2c接收数据
@api i2c.recv(id, addr, len)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int I2C子设备的地址, 7位地址
@int 接收数据的长度
@return string 收到的数据
@usage
-- 从i2c1读取2个字节的数据
local data = i2c.recv(1, 0x5C, 2)
*/
static int l_i2c_recv(lua_State *L)
{
    int id = 0;
    if (!lua_isuserdata(L, 1))
    {
        id = luaL_checkinteger(L, 1);
    }
    int addr = luaL_checkinteger(L, 2);
    int len = luaL_checkinteger(L, 3);
    char *buff = (char *)luat_heap_malloc(len);
    int result;
    if (lua_isuserdata(L, 1))
    {
        luat_ei2c *ei2c = toei2c(L);
        result = i2c_soft_recv(ei2c, addr, buff, len);
    }
    else
    {
        result = luat_i2c_recv(id, addr, buff, len);
    }
    if (result != 0)
    { //如果返回值不为0，说明收失败了
        len = 0;
        LLOGD("i2c receive result %d", result);
    }
    lua_pushlstring(L, buff, len);
    luat_heap_free(buff);
    return 1;
}

/*
i2c写寄存器数据
@api i2c.writeReg(id, addr, reg, data,stop)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int I2C子设备的地址, 7位地址
@int 寄存器地址
@string 待发送的数据
@integer 可选参数 是否发送停止位 1发送 0不发送 默认发送(105不支持)
@return true/false 发送是否成功
@usage
-- 从i2c1的地址为0x5C的设备的寄存器0x01写入2个字节的数据
i2c.writeReg(1, 0x5C, 0x01, string.char(0x00, 0xF2))
*/
static int l_i2c_write_reg(lua_State *L)
{
    int id = 0;
    if (!lua_isuserdata(L, 1))
    {
        id = luaL_checkinteger(L, 1);
    }
    int addr = luaL_checkinteger(L, 2);
    int reg = luaL_checkinteger(L, 3);
    size_t len;
    const char *lb = luaL_checklstring(L, 4, &len);
    int stop = luaL_optnumber(L, 5 , 1);
    char *buff = (char *)luat_heap_malloc(sizeof(char) * len + 1);
    *buff = (char)reg;
    memcpy(buff + 1, lb, sizeof(char) + len + 1);
    int result;
    if (lua_isuserdata(L, 1))
    {
        luat_ei2c *ei2c = toei2c(L);
        result = i2c_soft_send(ei2c, addr, buff, len + 1,stop);
    }
    else
    {
        result = luat_i2c_send(id, addr, buff, len + 1,stop);
    }
    luat_heap_free(buff);
    lua_pushboolean(L, result == 0);
    return 1;
}

/*
i2c读寄存器数据
@api i2c.readReg(id, addr, reg, len)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int I2C子设备的地址, 7位地址
@int 寄存器地址
@int 待接收的数据长度
@integer 可选参数 是否发送停止位 1发送 0不发送 默认发送(105不支持)
@return string 收到的数据
@usage
-- 从i2c1的地址为0x5C的设备的寄存器0x01读出2个字节的数据
i2c.readReg(1, 0x5C, 0x01, 2)
*/
static int l_i2c_read_reg(lua_State *L)
{
    int id = 0;
    if (!lua_isuserdata(L, 1))
    {
        id = luaL_checkinteger(L, 1);
    }
    int addr = luaL_checkinteger(L, 2);
    int reg = luaL_checkinteger(L, 3);
    int len = luaL_checkinteger(L, 4);
    int stop = luaL_optnumber(L, 5 , 0);
    char temp = (char)reg;
    int result;
    if (lua_isuserdata(L, 1))
    {
        luat_ei2c *ei2c = toei2c(L);
        result = i2c_soft_send(ei2c, addr, &temp, 1,stop);
    }
    else
    {
        result = luat_i2c_send(id, addr, &temp, 1,stop);
    }
    if (result != 0)
    { //如果返回值不为0，说明收失败了
        LLOGD("i2c send result %d", result);
        lua_pushlstring(L, NULL, 0);
        return 1;
    }
    char *buff = (char *)luat_heap_malloc(sizeof(char) * len);
    if (lua_isuserdata(L, 1))
    {
        luat_ei2c *ei2c = toei2c(L);
        result = i2c_soft_recv(ei2c, addr, buff, len);
    }
    else
    {
        result = luat_i2c_recv(id, addr, buff, len);
    }
    if (result != 0)
    { //如果返回值不为0，说明收失败了
        len = 0;
        LLOGD("i2c receive result %d", result);
    }
    lua_pushlstring(L, buff, len);
    luat_heap_free(buff);
    return 1;
}

/*
关闭i2c设备
@api i2c.close(id)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@return nil 无返回值
@usage
-- 关闭i2c1
i2c.close(1)
*/
static int l_i2c_close(lua_State *L)
{
    int id = luaL_checkinteger(L, 1);
    luat_i2c_close(id);
    return 0;
}

/*
从i2c总线读取DHT12的温湿度数据
@api i2c.readDHT12(id)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int DHT12的设备地址,默认0x5C
@return boolean 读取成功返回true,否则返回false
@return int 湿度值,单位0.1%, 例如 591 代表 59.1%
@return int 温度值,单位0.1摄氏度, 例如 292 代表 29.2摄氏度
@usage
-- 从i2c0读取DHT12
i2c.setup(0)
local re, H, T = i2c.readDHT12(0)
if re then
    log.info("dht12", H, T)
end
*/
static int l_i2c_readDHT12(lua_State *L)
{
    int id = 0;
    if (!lua_isuserdata(L, 1))
    {
        id = luaL_checkinteger(L, 1);
    }
    int addr = luaL_optinteger(L, 2, 0x5C);
    char buff[5] = {0};
    char temp = 0x00;
    int result = -1;
    if (lua_isuserdata(L, 1))
    {
        luat_ei2c *ei2c = toei2c(L);
        result = i2c_soft_send(ei2c, addr, &temp, 1,1);
    }
    else
    {
        result = luat_i2c_send(id, addr, &temp, 1,1);
    }
    if (result != 0)
    {
        LLOGD("DHT12 i2c bus write fail");
        lua_pushboolean(L, 0);
        return 1;
    }
    if (lua_isuserdata(L, 1))
    {
        luat_ei2c *ei2c = toei2c(L);
        result = i2c_soft_recv(ei2c, addr, buff, 5);
    }
    else
    {
        result = luat_i2c_recv(id, addr, buff, 5);
    }
    if (result != 0)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    else
    {
        if (buff[0] == 0 && buff[1] == 0 && buff[2] == 0 && buff[3] == 0 && buff[4] == 0)
        {
            LLOGD("DHT12 DATA emtry");
            lua_pushboolean(L, 0);
            return 1;
        }
        // 检查crc值
        LLOGD("DHT12 DATA %02X%02X%02X%02X%02X", buff[0], buff[1], buff[2], buff[3], buff[4]);
        uint8_t crc_act = (uint8_t)buff[0] + (uint8_t)buff[1] + (uint8_t)buff[2] + (uint8_t)buff[3];
        uint8_t crc_exp = (uint8_t)buff[4];
        if (crc_act != crc_exp)
        {
            LLOGD("DATA12 DATA crc not ok");
            lua_pushboolean(L, 0);
            return 1;
        }
        lua_pushboolean(L, 1);
        lua_pushinteger(L, (uint8_t)buff[0] * 10 + (uint8_t)buff[1]);
        if (((uint8_t)buff[2]) > 127)
            lua_pushinteger(L, ((uint8_t)buff[2] - 128) * -10 + (uint8_t)buff[3]);
        else
            lua_pushinteger(L, (uint8_t)buff[2] * 10 + (uint8_t)buff[3]);
        return 3;
    }
}

/*
从i2c总线读取DHT30的温湿度数据(由"好奇星"贡献)
@api i2c.readSHT30(id,addr)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int 设备addr,SHT30的设备地址,默认0x44 bit7
@return boolean 读取成功返回true,否则返回false
@return int 湿度值,单位0.1%, 例如 591 代表 59.1%
@return int 温度值,单位0.1摄氏度, 例如 292 代表 29.2摄氏度
@usage
-- 从i2c0读取SHT30
i2c.setup(0)
local re, H, T = i2c.readSHT30(0)
if re then
    log.info("sht30", H, T)
end
*/
static int l_i2c_readSHT30(lua_State *L)
{
    char buff[7] = {0x2c, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00};
    float temp = 0x00;
    float hum = 0x00;

    int result = -1;
    if (lua_isuserdata(L, 1))
    {
        luat_ei2c *ei2c = toei2c(L);
        i2c_soft_send(ei2c, ei2c->addr, buff, 2,1);
        luat_timer_mdelay(13);

        result = i2c_soft_recv(ei2c, ei2c->addr, buff, 6);
    }
    else
    {
        int id = luaL_optinteger(L, 1, 0);
        int addr = luaL_optinteger(L, 2, 0x44);

        luat_i2c_send(id, addr, &buff, 2,1);
        luat_timer_mdelay(1);
        result = luat_i2c_recv(id, addr, buff, 6);
    }

    if (result != 0)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    else
    {
        if (buff[0] == 0 && buff[1] == 0 && buff[2] == 0 && buff[3] == 0 && buff[4] == 0)
        {
            LLOGD("SHT30 DATA emtry");
            lua_pushboolean(L, 0);
            return 1;
        }
        // 检查crc值
        // LLOGD("SHT30 DATA %02X %02X %02X %02X %02X %02X", buff[0], buff[1], buff[2], buff[3], buff[4], buff[5]);

        temp = ((((buff[0] * 256) + buff[1]) * 175) / 6553.5) - 450;
        hum = ((((buff[3] * 256) + buff[4]) * 100) / 6553.5);

        // LLOGD("\r\n SHT30  %d deg  %d Hum ", temp, hum);
        //  跳过CRC

        // uint8_t crc_act = (uint8_t)buff[0] + (uint8_t)buff[1] + (uint8_t)buff[2] + (uint8_t)buff [3];
        // uint8_t crc_exp = (uint8_t)buff[4];
        // if (crc_act != crc_exp) {
        //     LLOGD("DATA12 DATA crc not ok");
        //     lua_pushboolean(L, 0);
        //     return 1;
        // }

        // Convert the data
        lua_pushboolean(L, 1);
        lua_pushinteger(L, (int)hum);
        lua_pushinteger(L, (int)temp);

        return 3;
        // 华氏度
        // fTemp = (cTemp * 1.8) + 32;
    }
}


int LUAT_WEAK luat_i2c_transfer(int id, int addr, uint8_t *reg, size_t reg_len, uint8_t *buff, size_t len)
{
    int result;
    result = luat_i2c_send(id, addr, reg, reg_len, 0);
    if (result != 0) return-1;
    return luat_i2c_recv(id, addr, buff, len);
}

/**
i2c通用传输，包括发送N字节，发送N字节+接收N字节，接收N字节三种功能，在发送转接收过程中发送reStart信号,解决类似mlx90614必须带restart信号，但是又不能用i2c.send来控制的，比如air105
@api i2c.transfer(id, addr, txBuff, rxBuff, rxLen)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int I2C子设备的地址, 7位地址
@integer/string/zbuff 待发送的数据,自适应参数类型，如果为nil，则不发送数据
@zbuff 待接收数据的zbuff 如果不用zbuff，则接收数据将在return返回
@int 需要接收的数据长度，如果为0或nil，则不接收数据
@return boolean true/false 发送是否成功
@return string or nil 如果参数5是interger，则返回接收到的数据
@usage
local result, _ = i2c.transfer(0, 0x11, txbuff, rxbuff, 1)
local result, _ = i2c.transfer(0, 0x11, txbuff, nil, 0)	--只发送txbuff里的数据，不接收数据，典型应用就是写寄存器了，这里寄存器地址和值都放在了txbuff里
local result, _ = i2c.transfer(0, 0x11, "\x01\x02\x03", nil, 1) --发送0x01， 0x02，0x03，不接收数据，如果是eeprom，就是往0x01的地址写02和03，或者往0x0102的地址写03，看具体芯片了
local result, rxdata = i2c.transfer(0, 0x11, "\x01\x02", nil, 1) --发送0x01， 0x02，然后接收1个字节，典型应用就是eeprom
local result, rxdata = i2c.transfer(0, 0x11, 0x00, nil, 1) --发送0x00，然后接收1个字节，典型应用各种传感器
*/
static int l_i2c_transfer(lua_State *L)
{
	int addr = luaL_checkinteger(L, 2);
	size_t tx_len = 0;
	size_t rx_len = 0;
	int result = 0;
	uint8_t temp[1];
	uint8_t *tx_buff = NULL;
	uint8_t *rx_buff = NULL;
	uint8_t tx_heap_flag = 0;
	if (lua_isnil(L, 3)) {
		tx_len = 0;
	}
	else if (lua_isinteger(L, 3)) {
		temp[0] = luaL_checkinteger(L, 3);
		tx_buff = temp;
		tx_len = 1;
	}
	else if (lua_isstring(L, 3)) {
		tx_buff = (uint8_t*)luaL_checklstring(L, 3, &tx_len);
	}
    else if (lua_istable(L, 3)) {
        const int tx_len = lua_rawlen(L, 3); //返回数组的长度
        tx_buff = (uint8_t *)luat_heap_malloc(tx_len);
        tx_heap_flag = 1;
        for (size_t i = 0; i < tx_len; i++)
        {
            lua_rawgeti(L, 3, 1 + i);
            tx_buff[i] = (char)lua_tointeger(L, -1);
            lua_pop(L, 1); //将刚刚获取的元素值从栈中弹出
        }
    }
	else {
		luat_zbuff_t *buff = ((luat_zbuff_t *)luaL_checkudata(L, 3, LUAT_ZBUFF_TYPE));
		tx_buff = buff->addr;
		tx_len = buff->used;
	}
	luat_zbuff_t *rbuff = ((luat_zbuff_t *)luaL_testudata(L, 4, LUAT_ZBUFF_TYPE));
	if (lua_isnil(L, 5)) {
		rx_len = 0;
	}
	else if (lua_isinteger(L, 5)) {
		rx_len = luaL_checkinteger(L, 5);
		if (rx_len) {
			if (!rbuff) {
				rx_buff = luat_heap_malloc(rx_len);
			}
			else {
				if ((rbuff->used + rx_len) > rbuff->len) {
					__zbuff_resize(rbuff, rbuff->len + rx_len);
				}
				rx_buff = rbuff->addr + rbuff->used;
			}
		}
	}

	int id = 0;
	if (!lua_isuserdata(L, 1)) {
		id = luaL_checkinteger(L, 1);
		if (rx_buff && rx_len) {
			result = luat_i2c_transfer(id, addr, tx_buff, tx_len, rx_buff, rx_len);
		} else {
			result = luat_i2c_transfer(id, addr, NULL, 0, tx_buff, tx_len);
		}
	}
	if (tx_heap_flag) {
		luat_heap_free(tx_buff);
	}
//	else if (lua_isuserdata(L, 1))
//    {
//        luat_ei2c *ei2c = toei2c(L);
//    }
	lua_pushboolean(L, !result);
	if (rx_buff && rx_len) {
		if (rbuff) {
			rbuff->used += rx_len;
			lua_pushnil(L);
		} else {
            lua_pushlstring(L, (const char *)rx_buff, rx_len);
            luat_heap_free(rx_buff);
		}
	} else {
		lua_pushnil(L);
	}
	return 2;

}

#include "rotable2.h"
static const rotable_Reg_t reg_i2c[] =
{
    { "exist",      ROREG_FUNC(l_i2c_exist)},
    { "setup",      ROREG_FUNC(l_i2c_setup)},
    { "createSoft", ROREG_FUNC(l_i2c_soft)},
#ifdef __F1C100S__
#else
    { "send",       ROREG_FUNC(l_i2c_send)},
    { "recv",       ROREG_FUNC(l_i2c_recv)},

#endif
	{ "transfer",	ROREG_FUNC(l_i2c_transfer)},
    { "writeReg",   ROREG_FUNC(l_i2c_write_reg)},
    { "readReg",    ROREG_FUNC(l_i2c_read_reg)},
    { "close",      ROREG_FUNC(l_i2c_close)},

    { "readDHT12",  ROREG_FUNC(l_i2c_readDHT12)},
    { "readSHT30",  ROREG_FUNC(l_i2c_readSHT30)},

    { "FAST",       ROREG_INT(1)},
    { "SLOW",       ROREG_INT(0)},
	{ NULL,         ROREG_INT(0) }
};

LUAMOD_API int luaopen_i2c(lua_State *L)
{
    luat_newlib2(L, reg_i2c);
    luaL_newmetatable(L, LUAT_EI2C_TYPE);
    lua_pop(L, 1);
    return 1;
}
