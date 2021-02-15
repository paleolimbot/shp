
#include <memory.h>
#include <R.h>
#include <Rinternals.h>
#include "minishp-shp.h"
#include "wk-v1.h"

#define HANDLE_CONTINUE_OR_BREAK(expr)                           \
    result = expr;                                               \
    if (result == WK_ABORT_FEATURE) continue; else if (result == WK_ABORT) break

typedef struct {
  SEXP shp_geometry;
  shp_file_t* shp;
  wk_handler_t* handler;
} shp_reader_t;

void shp_handle_geometry_point(shp_reader_t* reader, const wk_vector_meta_t* vector_meta) {
    wk_handler_t* handler = reader->handler;
    int result;

    int* indices = INTEGER(reader->shp_geometry);
    int size = Rf_length(reader->shp_geometry);

    wk_meta_t meta;
    WK_META_RESET(meta, WK_POINT);
    int has_z = vector_meta->flags & WK_FLAG_HAS_Z;
    if (has_z) meta.flags |= WK_FLAG_HAS_Z;

    meta.size = 0; // for now!

    // set on a per-feature basis
    int has_m = vector_meta->flags & WK_FLAG_HAS_M;
    if (has_m) meta.flags |= WK_FLAG_HAS_Z;

    wk_coord_t coord;

    for (int i = 0; i < size; i++) {
        HANDLE_CONTINUE_OR_BREAK(handler->feature_start(vector_meta, i, handler->handler_data));
        HANDLE_CONTINUE_OR_BREAK(handler->geometry_start(&meta, WK_PART_ID_NONE, handler->handler_data));

        if (meta.size > 0) {
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
    reader->shp = shp_open(filename);
    if (!shp_valid(reader->shp)) {
        Rf_error(reader->shp->error_buf);
    }

    wk_vector_meta_t vector_meta;
    WK_VECTOR_META_RESET(vector_meta, WK_POINT);
    vector_meta.size = Rf_length(reader->shp_geometry);
    
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

    if (reader->shp != NULL) {
        shp_close(reader->shp);
    }
}

SEXP shp_c_handle_geometry(SEXP shp_geometry, SEXP handler_xptr) {
    wk_handler_t* handler = (wk_handler_t*) R_ExternalPtrAddr(handler_xptr);
    shp_reader_t reader = { shp_geometry, NULL, handler };
    return R_ExecWithCleanup(
        &shp_handle_geometry_with_cleanup, 
        &reader,
        &shp_handle_geometry_cleanup, 
        &reader
    );
}
