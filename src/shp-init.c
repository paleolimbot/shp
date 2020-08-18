
#include <memory.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

extern char SALastError[1024];
SEXP shp_c_shapelib_version();
SEXP shp_c_file_meta(SEXP path);
SEXP shp_c_geometry_meta(SEXP path, SEXP indices);

static const R_CallMethodDef CallEntries[] = {
  {"shp_c_shapelib_version", (DL_FUNC) &shp_c_shapelib_version, 0},
  {"shp_c_file_meta", (DL_FUNC) &shp_c_file_meta, 1},
  {"shp_c_geometry_meta", (DL_FUNC) &shp_c_geometry_meta, 2},
  {NULL, NULL, 0}
};

void R_init_shp(DllInfo *dll) {
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);

  // initialize the last error message
  memset(SALastError, 0, 1024);
}

SEXP shp_c_shapelib_version() {
  SEXP out = PROTECT(Rf_allocVector(STRSXP, 1));
  SET_STRING_ELT(out, 0, Rf_mkChar("1.5.0"));
  UNPROTECT(1);
  return out;
}
