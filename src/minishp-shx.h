
#ifndef MINISHP_SHX_H
#define MINISHP_SHX_H

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "minishp-file.h"

#define SHX_ERROR_SIZE 1024

typedef struct {
    uint32_t offset;
    uint32_t content_length;
} shx_record_t;

typedef struct {
    void* file_handle;
    minishp_file_t file;
    char error_buf[SHX_ERROR_SIZE];
    uint32_t n_records;
    uint32_t cache_size;
    uint32_t cache_start;
    uint32_t cache_end;
    shx_record_t* cache;
} shx_file_t;

#ifdef __cplusplus
extern "C" {
#endif

shx_file_t* shx_open(const char* filename);
void shx_set_cache_size(shx_file_t* shx, size_t cache_size);
int shx_valid(shx_file_t* shx);
uint32_t shx_n_records(shx_file_t* shx);
size_t shx_record_n(shx_file_t* shx, shx_record_t* dest, uint32_t shape_id, size_t n);
shx_record_t* shx_record(shx_file_t* shx, uint32_t shape_id);
void shx_close(shx_file_t* shx);

#ifdef __cplusplus
}
#endif

#ifdef MINISHP_IMPL

// https://www.esri.com/Library/Whitepapers/Pdfs/Shapefile.pdf, page 23

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "minishp-port.h"

#define SHX_HEADER_SIZE 100

shx_file_t* shx_open(const char* filename) {
    shx_file_t* shx = (shx_file_t*) malloc(sizeof(shx_file_t));
    memset(shx->error_buf, 0, SHX_ERROR_SIZE);
    shx->n_records = UINT32_MAX;
    shx->file = minishp_file_default();
    shx->cache_size = 64;
    shx->cache_start = UINT32_MAX;
    shx->cache_end = UINT32_MAX;
    shx->cache = malloc(sizeof(shx_record_t) * shx->cache_size);

    shx->file_handle = shx->file.fopen(filename, "rb");
    if (shx->file_handle == NULL) {
        snprintf(shx->error_buf, SHX_ERROR_SIZE, "Failed to open shx file '%s'", filename);
    }

    return shx;
}

void shx_set_cache_size(shx_file_t* shx, size_t cache_size) {
    size_t previous = shx->cache_size;
    if (cache_size >= 1) {
        shx->cache_size = cache_size;
    } else {
        shx->cache_size = 1;
    }

    if (shx->cache_size != previous) {
        shx->cache_start = UINT32_MAX;
        shx->cache_end = UINT32_MAX;
        free(shx->cache);
        shx->cache = malloc(sizeof(shx_record_t) * shx->cache_size);
    }
}

int shx_valid(shx_file_t* shx) {
    return (shx != NULL) && (shx->file_handle != NULL);
}

uint32_t shx_n_records(shx_file_t* shx) {
    if (shx_valid(shx) && (shx->n_records == UINT32_MAX)) {
        shx->file.fseek(shx->file_handle, 0, SEEK_END);
        size_t shx_size = shx->file.ftell(shx->file_handle);
        shx->n_records = (shx_size - SHX_HEADER_SIZE) / sizeof(shx_record_t);
    }

    return shx->n_records;
}

size_t shx_record_n(shx_file_t* shx, shx_record_t* dest, uint32_t shape_id, size_t n) {
    if (!shx_valid(shx)) {
        return 0;
    }

    size_t shx_offset = SHX_HEADER_SIZE + sizeof(shx_record_t) * shape_id;
    if (shx->file.fseek(shx->file_handle, shx_offset, SEEK_SET) != 0) {
        snprintf(shx->error_buf, SHX_ERROR_SIZE, "Can't find shape_id '%u' in .shx", shape_id);
        return 0;
    }

    size_t n_read = shx->file.fread(dest, sizeof(shx_record_t), n, shx->file_handle);
    if (n_read != n) {
        snprintf(
            shx->error_buf, SHX_ERROR_SIZE, 
            "Expected %ul records in .shx but read %u", (uint32_t) n, (uint32_t) n_read
        );
    }

#ifdef IS_LITTLE_ENDIAN
    for (size_t i = 0; i < n_read; i++) {
        dest[i].offset = bswap_32(dest[i].offset);
        dest[i].content_length = bswap_32(dest[i].content_length);
    }
#endif

    return n_read;
}

shx_record_t* shx_record(shx_file_t* shx, uint32_t shape_id) {
    if (shape_id >= shx_n_records(shx)) {
        return NULL;
    }
    
    if ((shape_id < shx->cache_start) || (shape_id >= shx->cache_end)) {
        size_t n_read = shx_record_n(shx, shx->cache, shape_id, shx->cache_size);
        shx->cache_start = shape_id;
        shx->cache_end = shape_id + n_read;
    }

    return shx->cache + (shape_id - shx->cache_start);
}

void shx_close(shx_file_t* shx) {
    if (shx != NULL) {
        shx->file.fclose(shx->file_handle);
        free(shx->cache);
        free(shx);
    }
}

#endif

#endif
