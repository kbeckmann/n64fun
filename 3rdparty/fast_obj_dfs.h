// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#pragma once

#include "fast_obj.h"

void* fast_obj_dfs_file_open(const char* path, void* user_data);
void fast_obj_dfs_file_close(void* file, void* user_data);
size_t fast_obj_dfs_file_read(void* file, void* dst, size_t bytes, void* user_data);
unsigned long fast_obj_dfs_file_size(void* file, void* user_data);

static const fastObjCallbacks fast_obj_dfs_cb = {
    .file_open  = fast_obj_dfs_file_open,
    .file_close = fast_obj_dfs_file_close,
    .file_read  = fast_obj_dfs_file_read,
    .file_size  = fast_obj_dfs_file_size
};
