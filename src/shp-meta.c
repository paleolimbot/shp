
#include "shapefil.h"
#include "shp-common.h"
#include <memory.h>
#include <Rinternals.h>

SEXP shp_c_meta(SEXP path) {
  SHP_RESET_ERROR();

  const char* path0 = CHAR(STRING_ELT(path, 0));
  SEXP outType = PROTECT(Rf_allocVector(INTSXP, 1));
  SEXP outRecords = PROTECT(Rf_allocVector(INTSXP, 1));
  SEXP outBoundsMin = PROTECT(Rf_allocVector(REALSXP, 4));
  SEXP outBoundsMax = PROTECT(Rf_allocVector(REALSXP, 4));

  SHPHandle	hSHP = SHPOpen(path0, "rb");
  if (hSHP == NULL) {
    SHP_ERROR("%s", "SHPOpen: ");
  }

  INTEGER(outType)[0] = hSHP->nShapeType;
  INTEGER(outRecords)[0] = hSHP->nRecords;
  memcpy(REAL(outBoundsMin), hSHP->adBoundsMin, 4 * sizeof(double));
  memcpy(REAL(outBoundsMax), hSHP->adBoundsMax, 4 * sizeof(double));

  SHPClose(hSHP);

  const char *names[] = {"shp_type", "n_records", "bounds_min", "bounds_max", ""};
  SEXP out = PROTECT(Rf_mkNamed(VECSXP, names));
  SET_VECTOR_ELT(out, 0, outType);
  SET_VECTOR_ELT(out, 1, outRecords);
  SET_VECTOR_ELT(out, 2, outBoundsMin);
  SET_VECTOR_ELT(out, 3, outBoundsMax);

  UNPROTECT(5);
  return out;
}
