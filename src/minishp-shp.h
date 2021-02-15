
#ifndef MINISHP_SHP_H
#define MINISHP_SHP_H

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "minishp-shx.h"
#include "minishp-file.h"

enum shp_shape_type {
    SHP_TYPE_NULL = 0,
    SHP_TYPE_POINT = 1,
    SHP_TYPE_POLYLINE = 3,
    SHP_TYPE_POLYGON = 5,
    SHP_TYPE_MULTIPOINT = 8,
    SHP_TYPE_POINTZ = 11,
    SHP_TYPE_POLYLINEZ = 13,
    SHP_TYPE_POLYGONZ = 15,
    SHP_TYPE_MULTIPOINTZ = 18,
    SHP_TYPE_POINTM = 21,
    SHP_TYPE_POLYLINEM = 23,
    SHP_TYPE_POLYGONM = 25,
    SHP_TYPE_MULTIPOINTM = 28,
    SHP_TYPE_MULTIPATCH = 31
};

typedef struct {
    uint32_t file_code;
    uint32_t unused[5];
    uint32_t file_length;
    uint32_t version;
    uint32_t shape_type;
    unsigned char xmin[8];
    unsigned char ymin[8];
    unsigned char xmax[8];
    unsigned char ymax[8];
    unsigned char zmin[8];
    unsigned char zmax[8];
    unsigned char mmin[8];
    unsigned char mmax[8];
} shp_header_t;

typedef struct {
    uint32_t record_number;
    uint32_t content_length;
} shp_shape_header_t;

typedef struct {
    uint32_t shape_type;
    unsigned char x[8];
    unsigned char y[8];
} shp_shape_point_t;

typedef struct {
    uint32_t shape_type;
    unsigned char x[8];
    unsigned char y[8];
    unsigned char m[8];
} shp_shape_pointm_t;

typedef struct {
    uint32_t shape_type;
    unsigned char x[8];
    unsigned char y[8];
    unsigned char z[8];
    unsigned char m[8];
} shp_shape_pointz_t;

typedef struct {
    uint32_t record_number;
    uint32_t content_length;
    uint32_t shape_type;
    unsigned char x[8];
    unsigned char y[8];
} shp_shape_point_record_t;

typedef struct {
    uint32_t record_number;
    uint32_t content_length;
    uint32_t shape_type;
    unsigned char x[8];
    unsigned char y[8];
    unsigned char m[8];
} shp_shape_pointm_record_t;

typedef struct {
    uint32_t record_number;
    uint32_t content_length;
    uint32_t shape_type;
    unsigned char x[8];
    unsigned char y[8];
    unsigned char z[8];
    unsigned char m[8];
} shp_shape_pointz_record_t;

#define SHP_ERROR_SIZE 1024

typedef struct {
    void* file_handle;
    minishp_file_t file;
    char error_buf[SHP_ERROR_SIZE];
    shx_file_t* shx;
    shp_header_t header;
} shp_file_t;

shp_file_t* shp_open(const char* filename);
int shp_valid(shp_file_t* shp);
void shp_close(shp_file_t* shp);

#ifdef MINISHP_IMPL

#include "minishp-port.h"
#include <stdlib.h>
#include <memory.h>

shp_file_t* shp_open(const char* filename) {
    shp_file_t* shp = (shp_file_t*) malloc(sizeof(shp_file_t));
    memset(shp->error_buf, 0, SHP_ERROR_SIZE);
    shp->file = minishp_file_default();
    shp->shx = NULL;

    shp->file_handle = shp->file.fopen(filename, "rb");
    if (shp->file_handle == NULL) {
        snprintf(shp->error_buf, SHP_ERROR_SIZE, "Failed to open shp file '%s'", filename);
        return shp;
    }

    size_t n_read = shp->file.fread(&shp->header, sizeof(shp_header_t), 1, shp->file_handle);
    if (n_read != 1) {
        snprintf(shp->error_buf, SHP_ERROR_SIZE, "Failed to read header from '%s'", filename);
        shp->file.fclose(shp->file_handle);
        shp->file_handle = NULL;
        return shp;
    }

#ifdef IS_LITTLE_ENDIAN
    shp->header.file_code = bswap_32(shp->header.file_code);
    shp->header.file_length = bswap_32(shp->header.file_length);
#endif

    if (shp->header.file_code != 9994) {
        snprintf(
            shp->error_buf, SHP_ERROR_SIZE, 
            "Expected file code 9994 but found %u", shp->header.file_code
        );
        shp->file.fclose(shp->file_handle);
        shp->file_handle = NULL;
        return shp;
    }

    if (shp->header.version != 1000) {
        snprintf(
            shp->error_buf, SHP_ERROR_SIZE, 
            "Expected file version 1000 but found %u", shp->header.version
        );
        shp->file.fclose(shp->file_handle);
        shp->file_handle = NULL;
        return shp;
    }

    return shp;
}

int shp_valid(shp_file_t* shp) {
    return shp->file_handle != NULL;
}

void shp_close(shp_file_t* shp) {
    if (shp != NULL) {
        shp->file.fclose(shp->file_handle);
        free(shp);
    }
}

size_t shp_read_pointz_record(shp_file_t* shp, shp_shape_pointz_record_t* dest, size_t n) {
    size_t n_read = shp->file.fread(
        dest, 
        sizeof(shp_shape_pointz_record_t), 
        n, 
        shp->file_handle
    );

    for (size_t i = 0; i < n_read; i++) {
#ifdef IS_LITTLE_ENDIAN
        dest[i].record_number = bswap_32(dest[i].record_number);
        dest[i].content_length = bswap_32(dest[i].content_length);
#else
        dest[i].shape_type = bswap_32(dest[i].shape_type);
        // needs some thinking about how to swap doubles
#endif
    }

    return n_read;
}

#endif

#endif
