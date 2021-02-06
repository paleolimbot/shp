// Generated by cpp11: do not edit by hand
// clang-format off


#include "cpp11/declarations.hpp"

// dbf.cpp
list cpp_read_dbf_meta(std::string filename);
extern "C" SEXP _shp_cpp_read_dbf_meta(SEXP filename) {
  BEGIN_CPP11
    return cpp11::as_sexp(cpp_read_dbf_meta(cpp11::as_cpp<cpp11::decay_t<std::string>>(filename)));
  END_CPP11
}
// dbf.cpp
void cpp_read_dbf(std::string filename);
extern "C" SEXP _shp_cpp_read_dbf(SEXP filename) {
  BEGIN_CPP11
    cpp_read_dbf(cpp11::as_cpp<cpp11::decay_t<std::string>>(filename));
    return R_NilValue;
  END_CPP11
}

extern "C" {
/* .Call calls */
extern SEXP _shp_cpp_read_dbf(SEXP);
extern SEXP _shp_cpp_read_dbf_meta(SEXP);
extern SEXP shp_c_file_meta(SEXP);
extern SEXP shp_c_geometry_meta(SEXP, SEXP);
extern SEXP shp_c_shapelib_version();

static const R_CallMethodDef CallEntries[] = {
    {"_shp_cpp_read_dbf",      (DL_FUNC) &_shp_cpp_read_dbf,      1},
    {"_shp_cpp_read_dbf_meta", (DL_FUNC) &_shp_cpp_read_dbf_meta, 1},
    {"shp_c_file_meta",        (DL_FUNC) &shp_c_file_meta,        1},
    {"shp_c_geometry_meta",    (DL_FUNC) &shp_c_geometry_meta,    2},
    {"shp_c_shapelib_version", (DL_FUNC) &shp_c_shapelib_version, 0},
    {NULL, NULL, 0}
};
}

extern "C" void R_init_shp(DllInfo* dll){
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
}