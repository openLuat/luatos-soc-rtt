
#include "luat_base.h"
#include "luat_fs.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_file.h>

#define LUAT_LOG_TAG "fs"
#include "luat_log.h"

#define TAG "luat.fs"

#ifdef LUA_USE_VFS_FILENAME_OFFSET
#define FILENAME_OFFSET (filename[0] == '/' ? 1 : 0)
#else
#define FILENAME_OFFSET 0
#endif

#ifdef LUAT_USE_FS_VFS

FILE* luat_vfs_dfs_fopen(void* userdata, const char *filename, const char *mode) {
    dfs_fd *file = (dfs_fd*)luat_heap_malloc(sizeof(dfs_fd));
    int flag = 0;
    for (size_t i = 0; i < strlen(mode); i++)
    {
        char m = *(mode + i);
        switch (m)
        {
        case 'r':
            flag |= O_RDONLY;
            break;
        case 'w':
            flag |= O_RDWR | O_CREAT | O_TRUNC;
            break;
        case 'a':
            flag |= O_APPEND;
            break;
        case '+':
            flag |= O_APPEND;
            break;
        
        default:
            break;
        }
    }
    int ret = dfs_file_open(file, filename + FILENAME_OFFSET, flag);
    if (ret < 0) {
        luat_heap_free(file);
        return 0;
    }
    return (FILE*)file;
}

int luat_vfs_dfs_getc(void* userdata, FILE* stream) {
    //LLOGD("dfs_getc %p", stream);
    dfs_fd* file = (dfs_fd*)stream;
    char buff = 0;
    int ret = dfs_file_read(file, (void*)&buff, 1);
    if (ret < 0)
        return -1;
    return (int)buff;
}

int luat_vfs_dfs_fseek(void* userdata, FILE* stream, long int offset, int origin) {
    dfs_fd* file = (dfs_fd*)stream;
    int ret = dfs_file_lseek(file, offset);
    return ret < 0 ? -1 : 0;
}

int luat_vfs_dfs_ftell(void* userdata, FILE* stream) {
    return ftell(stream);
}

int luat_vfs_dfs_fclose(void* userdata, FILE* stream) {
    dfs_fd* file = (dfs_fd*)stream;
    int ret = dfs_file_close(file);
    if (ret < 0) 
        luat_heap_free(file);
    return 0;
}

int luat_vfs_dfs_feof(void* userdata, FILE* stream) {
    return feof(stream);
}

int luat_vfs_dfs_ferror(void* userdata, FILE* stream) {
    return ferror(stream);
}

size_t luat_vfs_dfs_fread(void* userdata, void *ptr, size_t size, size_t nmemb, FILE* stream) {
    dfs_fd* file = (dfs_fd*)stream;
    int ret = dfs_file_read(file, ptr, size*nmemb);
    return ret < 0 ? 0 : ret;
}

size_t luat_vfs_dfs_fwrite(void* userdata, const void *ptr, size_t size, size_t nmemb, FILE* stream) {
    dfs_fd* file = (dfs_fd*)stream;
    int ret = dfs_file_write(file, ptr, size*nmemb);
    return ret < 0 ? 0 : ret;
}

int luat_vfs_dfs_remove(void* userdata, const char *filename) {
    return dfs_file_unlink(filename + FILENAME_OFFSET);
}

int luat_vfs_dfs_rename(void* userdata, const char *old_filename, const char *new_filename) {
#if LUA_USE_VFS_FILENAME_OFFSET
    return dfs_file_rename(old_filename + (old_filename[0] == '/' ? 1 : 0), new_filename + (new_filename[0] == '/' ? 1 : 0));
#else
    return dfs_file_rename(old_filename, new_filename);
#endif
}

int luat_vfs_dfs_fexist(void* userdata, const char *filename) {
    int fd = luat_vfs_dfs_fopen(userdata, filename, "rb");
    if (fd) {
        luat_vfs_dfs_fclose(userdata, fd);
        return 1;
    }
    return 0;
}

size_t luat_vfs_dfs_fsize(void* userdata, const char *filename) {
    int fd;
    size_t size = 0;
    fd = luat_vfs_dfs_fopen(userdata, filename, "rb");
    if (fd) {
        luat_vfs_dfs_fseek(userdata, fd, 0, SEEK_END);
        size = luat_vfs_dfs_ftell(userdata, fd); 
        luat_vfs_dfs_fclose(userdata, fd);
    }
    return size;
}

int luat_vfs_dfs_mkfs(void* userdata, luat_fs_conf_t *conf) {
    LLOGE("not support yet : mkfs");
    return -1;
}
int luat_vfs_dfs_mount(void** userdata, luat_fs_conf_t *conf) {
    //LLOGE("not support yet : mount");
    return 0;
}
int luat_vfs_dfs_umount(void* userdata, luat_fs_conf_t *conf) {
    //LLOGE("not support yet : umount");
    return 0;
}

int luat_vfs_dfs_mkdir(void* userdata, char const* _DirName) {
    //LLOGE("not support yet : mkdir");
    return -1;
}
int luat_vfs_dfs_rmdir(void* userdata, char const* _DirName) {
    //LLOGE("not support yet : rmdir");
    return -1;
}
int luat_vfs_dfs_info(void* userdata, const char* path, luat_fs_info_t *conf) {

    memcpy(conf->filesystem, "dfs", strlen("dfs")+1);
    conf->type = 0;
    conf->total_block = 0;
    conf->block_used = 0;
    conf->block_size = 512;
    return 0;
}

#define T(name) .name = luat_vfs_dfs_##name
const struct luat_vfs_filesystem vfs_fs_dfs = {
    .name = "dfs",
    .opts = {
        T(mkfs),
        T(mount),
        T(umount),
        T(mkdir),
        T(rmdir),
        T(remove),
        T(rename),
        T(fsize),
        T(fexist),
        T(info)
    },
    .fopts = {
        T(fopen),
        T(getc),
        T(fseek),
        T(ftell),
        T(fclose),
        T(feof),
        T(ferror),
        T(fread),
        T(fwrite)
    }
};

#endif



