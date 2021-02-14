
#include "shapefil.h"
#include "shp-common.h"
#include <memory.h>
#include <R.h>
#include <Rinternals.h>
#include "wk-v1.h"

int shp_wk_type_from_shp_type(int shp_type) {
    switch (shp_type) {
    case SHPT_NULL:
        return WK_GEOMETRY;
    case SHPT_POINT:
    case SHPT_POINTM:
    case SHPT_POINTZ: 
        return WK_POINT;
    case SHPT_ARC:
    case SHPT_ARCM:
    case SHPT_ARCZ:
        return WK_MULTILINESTRING;
    case SHPT_POLYGON:
    case SHPT_POLYGONM:
    case SHPT_POLYGONZ:
        return WK_POLYGON;
    case SHPT_MULTIPOINT:
    case SHPT_MULTIPOINTM:
    case SHPT_MULTIPOINTZ:
        return WK_MULTIPOINT;
    case SHPT_MULTIPATCH:
        return WK_MULTIPOLYGON;
    default:
        Rf_error("Can't handle shapefile type '%d'", shp_type);
    }
}

int shp_wk_flags_from_shp_type(int shp_type) {
    int flags = 0;
    flags |= WK_FLAG_HAS_BOUNDS;

    switch (shp_type) {
    case SHPT_NULL:
    case SHPT_MULTIPATCH:
        flags |= WK_FLAG_DIMS_UNKNOWN;
        break;
    case SHPT_POINT:
    case SHPT_ARC:
    case SHPT_POLYGON:
    case SHPT_MULTIPOINT:
        break;
    case SHPT_POINTM:
    case SHPT_ARCM:
    case SHPT_POLYGONM:
    case SHPT_MULTIPOINTM:
        flags |= WK_FLAG_HAS_M;
        break;
    case SHPT_POINTZ:
    case SHPT_ARCZ:
    case SHPT_POLYGONZ:
    case SHPT_MULTIPOINTZ:
        flags |= WK_FLAG_HAS_M;
        flags |= WK_FLAG_HAS_Z;
        break;
    default:
        Rf_error("Can't handle shapefile type '%d'", shp_type);
    }

    return flags;
}

#define HANDLE_CONTINUE_OR_BREAK(expr)                           \
    result = expr;                                               \
    if (result == WK_ABORT_FEATURE) continue; else if (result == WK_ABORT) break

typedef struct {
  SEXP shp_geometry;
  SHPHandle	hSHP;
  SHPObject* hObj;
  wk_handler_t* handler;
} shp_reader_t;

void shp_handle_geometry_point(shp_reader_t* reader, const wk_vector_meta_t* vector_meta) {
    wk_handler_t* handler = reader->handler;
    int result;

    SHPSetFastModeReadObject(reader->hSHP, 1);

    int* indices = INTEGER(reader->shp_geometry);
    int size = Rf_length(reader->shp_geometry);

    wk_meta_t meta;
    WK_META_RESET(meta, WK_POINT);
    int has_z = vector_meta->flags & WK_FLAG_HAS_Z;
    if (has_z) meta.flags |= WK_FLAG_HAS_Z;

    // set on a per-feature basis
    int has_m = vector_meta->flags & WK_FLAG_HAS_M;
    if (has_m) meta.flags |= WK_FLAG_HAS_Z;

    wk_coord_t coord;

    for (int i = 0; i < size; i++) {
        if (reader->hObj != NULL) {
            SHPDestroyObject(reader->hObj);
            reader->hObj = NULL;
        }

        reader->hObj = SHPReadObject(reader->hSHP, indices[i]);
        if (reader->hObj == NULL) {
            result = handler->error(
                i, 
                WK_DEFAULT_ERROR_CODE, 
                "Failed to read object", 
                handler->handler_data
            );
            if (result == WK_ABORT) {
                break;
            } else {
                continue;
            }
        }

        meta.size = reader->hObj->nVertices;

        // in fast mode, bMeasureIsUsed is not a thing, but we can use the definition
        // of a no-data value in the shapefile spec, which is anything < -1e38
        if ((reader->hObj->nVertices > 0) && (reader->hObj->padfM[0] > -1e38)) {
            has_m = 1;
            meta.flags |= WK_FLAG_HAS_M;
        } else {
            has_m = 0;
            meta.flags &= ~WK_FLAG_HAS_M;
        }

        HANDLE_CONTINUE_OR_BREAK(handler->feature_start(vector_meta, i, handler->handler_data));
        HANDLE_CONTINUE_OR_BREAK(handler->geometry_start(&meta, WK_PART_ID_NONE, handler->handler_data));

        if (reader->hObj->nVertices > 0) {
            coord.v[0] = reader->hObj->padfX[0];
            coord.v[1] = reader->hObj->padfY[0];
            if (has_z && has_m) {
                coord.v[2] = reader->hObj->padfZ[0];
                coord.v[3] = reader->hObj->padfM[0];
            } else if (has_z) {
                coord.v[2] = reader->hObj->padfZ[0];
            } else if (has_m) {
                coord.v[2] = reader->hObj->padfM[0];
            }

            HANDLE_CONTINUE_OR_BREAK(handler->coord(&meta, coord, 0, handler->handler_data));
        }

        HANDLE_CONTINUE_OR_BREAK(handler->geometry_end(&meta, WK_PART_ID_NONE, handler->handler_data));
        HANDLE_CONTINUE_OR_BREAK(handler->feature_end(vector_meta, i, handler->handler_data));
    }
}

SEXP shp_handle_geometry_with_cleanup(void* data) {
    shp_reader_t* reader = (shp_reader_t*) data;
    if (reader->handler->api_version != 1) {
        Rf_error("Can't run a wk_handler with api_version '%d'", reader->handler->api_version);
    }
    reader->handler->initialize(&(reader->handler->dirty), reader->handler->handler_data);

    // Open the file
    SEXP shp_file = Rf_getAttrib(reader->shp_geometry, Rf_install("file"));
    const char* filename = Rf_translateCharUTF8(STRING_ELT(shp_file, 0));
    reader->hSHP = SHPOpen(filename, "rb");
    if (reader->hSHP == NULL) {
        SHP_ERROR("%s", "SHPOpen: ");
    }

    int n_records, type;
    double bounds_min[4], bounds_max[4];
    SHPGetInfo(reader->hSHP, &n_records, &type, bounds_min, bounds_max);

    wk_vector_meta_t vector_meta;
    WK_VECTOR_META_RESET(vector_meta, shp_wk_type_from_shp_type(type));
    vector_meta.size = Rf_length(reader->shp_geometry);
    vector_meta.flags = shp_wk_flags_from_shp_type(type);
    memcpy(vector_meta.bounds_min, bounds_min, sizeof(double) * 4);
    memcpy(vector_meta.bounds_max, bounds_max, sizeof(double) * 4);

    int result;
    result = reader->handler->vector_start(&vector_meta, reader->handler->handler_data);
    if (result == WK_ABORT) {
        return reader->handler->vector_end(&vector_meta, reader->handler->handler_data);
    }

    switch (vector_meta.geometry_type) {
    case WK_POINT:
        shp_handle_geometry_point(reader, &vector_meta);
        break;
    default:
        Rf_error("Can't handle geometry type '%d'", vector_meta.geometry_type);
    }

    return reader->handler->vector_end(&vector_meta, reader->handler->handler_data);
}

void shp_handle_geometry_cleanup(void* data) {
    shp_reader_t* reader = (shp_reader_t*) data;

    reader->handler->deinitialize(reader->handler->handler_data);

    if (reader->hObj != NULL) {
        SHPDestroyObject(reader->hObj);
    }
    if (reader->hSHP != NULL) {
        SHPClose(reader->hSHP);
    }
}

SEXP shp_c_handle_geometry(SEXP shp_geometry, SEXP handler_xptr) {
    wk_handler_t* handler = (wk_handler_t*) R_ExternalPtrAddr(handler_xptr);
    shp_reader_t reader = { shp_geometry, NULL, NULL, handler };
    return R_ExecWithCleanup(
        &shp_handle_geometry_with_cleanup, 
        &reader,
        &shp_handle_geometry_cleanup, 
        &reader
    );
}
