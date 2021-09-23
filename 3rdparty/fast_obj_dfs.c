// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include "fast_obj.h"
#include "libdragon.h"

#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#include <stdio.h>
#define DBG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DBG(...)
#endif

void* fast_obj_dfs_file_open(const char* path, void* user_data)
{
    DBG("%s(%s, %p)=", __FUNCTION__, path, user_data);
    int ret = dfs_open(path);
    DBG("%d\n", ret);

    // fast_obj doesn't handle error codes
    if (ret < 0) {
        return NULL;
    }

    return (void *) ret;
}

void fast_obj_dfs_file_close(void* file, void* user_data)
{
    DBG("%s(%p)=", __FUNCTION__, file);

    int ret = dfs_close((uint32_t) file);
    (void) ret;

    DBG("%d\n", ret);
}

size_t fast_obj_dfs_file_read(void* file, void* dst, size_t bytes, void* user_data)
{
    DBG("%s(%p, %p, %d, %p)=", __FUNCTION__, file, dst, bytes, user_data);
    size_t ret = dfs_read(dst, 1, bytes, (uint32_t) file);
    DBG("%d\n", ret);

    return ret;
}

unsigned long fast_obj_dfs_file_size(void* file, void* user_data)
{
    DBG("%s(%p, %p)=", __FUNCTION__, file, user_data);
    unsigned long ret = dfs_size((uint32_t) file);
    DBG("%ld\n",ret);

    return ret;
}

const fastObjCallbacks fast_obj_dfs_cb = {
    .file_open  = fast_obj_dfs_file_open,
    .file_close = fast_obj_dfs_file_close,
    .file_read  = fast_obj_dfs_file_read,
    .file_size  = fast_obj_dfs_file_size
};
