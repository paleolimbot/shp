
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

SEXP shp_c_shapelib_version();

static const R_CallMethodDef CallEntries[] = {
  {"shp_c_shapelib_version", (DL_FUNC) &shp_c_shapelib_version, 0},
  {NULL, NULL, 0}
};

void R_init_shp(DllInfo *dll) {
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
}

SEXP shp_c_shapelib_version() {
  SEXP out = PROTECT(Rf_allocVector(STRSXP, 1));
  SET_STRING_ELT(out, 0, Rf_mkChar("1.5.0"));
  UNPROTECT(1);
  return out;
}
