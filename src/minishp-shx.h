
#ifndef MINISHP_SHX_H
#define MINISHP_SHX_H

#ifdef __cplusplus
#include <cstdint>
#include <cstdio>
#else
#include <stdint.h>
#include <stdio.h>
#endif

#define SHX_ERROR_SIZE 1024

typedef struct {
    FILE* file;
    char error_buf[SHX_ERROR_SIZE];
    uint32_t n_records;
} shx_file_t;

typedef struct {
    uint32_t offset;
    uint32_t content_length;
} shx_record_t;

#ifdef __cplusplus
extern "C" {
#endif

shx_file_t* shx_open(const char* filename);
int shx_valid(shx_file_t* shx);
uint32_t shx_n_records(shx_file_t* shx);
size_t shx_record_n(shx_file_t* shx, shx_record_t* dest, uint32_t shape_id, size_t n);
shx_record_t shx_record(shx_file_t* shx, uint32_t shape_id);
void shx_close(shx_file_t* shx);

#ifdef __cplusplus
}
#endif

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

    shx->file = fopen(filename, "rb");
    if (shx->file == NULL) {
        snprintf(shx->error_buf, SHX_ERROR_SIZE, "Failed to open shx file '%s'", filename);
    }

    return shx;
}

int shx_valid(shx_file_t* shx) {
    return shx->file != NULL;
}

uint32_t shx_n_records(shx_file_t* shx) {
    if (shx_valid(shx) && (shx->n_records == UINT32_MAX)) {
        fseek(shx->file, 0, SEEK_END);
        size_t shx_size = ftell(shx->file);
        shx->n_records = (shx_size - SHX_HEADER_SIZE) / sizeof(shx_record_t);
    }

    return shx->n_records;
}

size_t shx_record_n(shx_file_t* shx, shx_record_t* dest, uint32_t shape_id, size_t n) {
    if (!shx_valid(shx)) {
        return 0;
    }

    size_t shx_offset = SHX_HEADER_SIZE + sizeof(shx_record_t) * shape_id;
    if (fseek(shx->file, shx_offset, SEEK_SET) != 0) {
        snprintf(shx->error_buf, SHX_ERROR_SIZE, "Can't find shape_id '%u' in .shx", shape_id);
        return 0;
    }

    size_t n_read = fread(dest, sizeof(shx_record_t), 1, shx->file);
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

shx_record_t shx_record(shx_file_t* shx, uint32_t shape_id) {
    shx_record_t output = { UINT32_MAX, 0};
    shx_record_n(shx, &output, shape_id, 1);
    return output;
}

void shx_close(shx_file_t* shx) {
    if (shx != NULL) {
        fclose(shx->file);
        free(shx);
    }
}

#endif
