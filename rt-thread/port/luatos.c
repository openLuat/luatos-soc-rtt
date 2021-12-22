/*
 * Copyright (c) 2019 Winner Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-02-13     tyx          first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "luat_base.h"

static void luatos(void* param) {
    rt_thread_mdelay(100); // 故意延后100ms
    luat_main();
    while (1)
        rt_thread_delay(10000000);
}

int rtt_luatos_init(void)
{
    rt_thread_t t = rt_thread_create("luat", luatos, RT_NULL, 8*1024, 15, 20);
    rt_thread_startup(t);
    return 0;
}
INIT_APP_EXPORT(rtt_luatos_init);

