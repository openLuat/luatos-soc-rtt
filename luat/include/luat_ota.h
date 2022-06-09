
#include "luat_base.h"

#ifndef LUAT_OTA_H
#define LUAT_OTA_H

#ifndef UPDATE_TGZ_PATH
#define UPDATE_TGZ_PATH "/update.tgz"
#endif

#ifndef UPDATE_BIN_PATH
#define UPDATE_BIN_PATH "/update.bin"
#endif

#define ROLLBACK_MARK_PATH "/rollback_mark"
#define UPDATE_MARK "/update_mark"
#define FLASHX_PATH "/flashx.bin"

#ifndef LUAT_OTA_MODE
#define LUAT_OTA_MODE 1
#endif

#ifndef LUAT_EXIT_REBOOT_DELAY
#define LUAT_EXIT_REBOOT_DELAY 15000
#endif

int luat_ota_update_or_rollback(void);
void luat_ota_reboot(int timeout_ms);

int luat_ota(uint32_t luadb_addr);
int luat_flash_write(uint32_t addr, uint8_t * buf, uint32_t len);

#endif
