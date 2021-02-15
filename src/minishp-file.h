
#ifndef MINISHP_FILE_H
#define MINISHP_FILE_H

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

typedef struct {
    void* (*fopen)(const char* filename, const char* mode);
    void (*fclose)(void* handle);
    size_t (*fread)(void* dest, size_t size, size_t n, void* handle);
    int (*fseek)(void* handle, long offset, int whence);
    long (*ftell)(void* handle);
} minishp_file_t;

#ifdef __cplusplus
extern "C" {
#endif
minishp_file_t minishp_file_default();
#ifdef __cplusplus
}
#endif

#ifdef MINISHP_IMPL

#include <stdlib.h>

typedef struct {
    FILE* file;
} minishp_file_default_t;

void* minishp_file_default_fopen(const char* filename, const char* mode) {
    minishp_file_default_t* file = (minishp_file_default_t*) malloc(sizeof(minishp_file_default_t));
    file->file = fopen(filename, mode);
    if (file->file == NULL) {
        free(file);
        return NULL;
    } else {
        return file;
    }
}

void minishp_file_default_fclose(void* handle) {
    minishp_file_default_t* file = (minishp_file_default_t*) handle;
    if (file == NULL) {
        return;
    }
    
    if (file->file != NULL) {
        fclose(file->file);
    }
    
    free(file);
}

size_t minishp_file_default_fread(void* dest, size_t size, size_t n, void* handle) {
    minishp_file_default_t* file = (minishp_file_default_t*) handle;
    return fread(dest, size, n, file->file);
}

int minishp_file_default_fseek(void* handle, long offset, int whence) {
    minishp_file_default_t* file = (minishp_file_default_t*) handle;
    return fseek(file->file, offset, whence);
}

long minishp_file_default_ftell(void* handle) {
    minishp_file_default_t* file = (minishp_file_default_t*) handle;
    return ftell(file->file);
}

minishp_file_t minishp_file_default() {
    minishp_file_t file = {
        &minishp_file_default_fopen,
        &minishp_file_default_fclose,
        &minishp_file_default_fread,
        &minishp_file_default_fseek,
        &minishp_file_default_ftell
    };

    return file;
}

#endif

#endif
