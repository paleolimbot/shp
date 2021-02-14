
#include <R.h>
#include <Rinternals.h>
#include "minishp-shx.h"

SEXP shp_c_shx_meta(SEXP filename) {
    const char* filename_utf8 = Rf_translateCharUTF8(STRING_ELT(filename, 0));
    int n_features = NA_INTEGER;
    shx_file_t* shx = shx_open(filename_utf8);
    if (!shx_valid(shx)) {
        shx_close(shx);
        Rf_error(shx->error_buf);
    }

    n_features = (int) shx_n_records(shx);

    shx_close(shx);

    const char* names[] = {"n_features", ""};
    SEXP output = PROTECT(Rf_mkNamed(VECSXP, names));
    SET_VECTOR_ELT(output, 0, Rf_ScalarInteger(n_features));
    UNPROTECT(1);
    return output;

}

SEXP shp_c_read_shx(SEXP filename, SEXP indices_sexp) {
    const char* filename_utf8 = Rf_translateCharUTF8(STRING_ELT(filename, 0));
    int size = Rf_length(indices_sexp);
    int* indices = INTEGER(indices_sexp);
    SEXP offset_sexp = PROTECT(Rf_allocVector(INTSXP, size));
    SEXP content_length_sexp = PROTECT(Rf_allocVector(INTSXP, size));
    int* offset = INTEGER(offset_sexp);
    int* content_length = INTEGER(content_length_sexp);

    shx_file_t* shx = shx_open(filename_utf8);
    if (!shx_valid(shx)) {
        shx_close(shx);
        Rf_error(shx->error_buf);
    }

    shx_record_t record;
    size_t n_read;
    for (int i = 0; i < size; i++) {
        n_read = shx_record_n(shx, &record, indices[i], 1);
        if (n_read == 0) {
            offset[i] = NA_INTEGER;
            content_length[i] = NA_INTEGER;
        } else {
            offset[i] = record.offset;
            content_length[i] = record.content_length;
        }
    }

    shx_close(shx);

    const char* names[] = {"offset", "content_length", ""};
    SEXP output = PROTECT(Rf_mkNamed(VECSXP, names));
    SET_VECTOR_ELT(output, 0, offset_sexp);
    SET_VECTOR_ELT(output, 1, content_length_sexp);
    UNPROTECT(3);
    return output;
}
