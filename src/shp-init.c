
#include <R.h>
#include <Rinternals.h>
#include <memory.h>

extern char SALastError[1024];
SEXP shp_c_shapelib_version();

SEXP shp_c_shapelib_version() {
  SEXP out = PROTECT(Rf_allocVector(STRSXP, 1));
  SET_STRING_ELT(out, 0, Rf_mkChar("1.5.0"));
  UNPROTECT(1);
  return out;
}
