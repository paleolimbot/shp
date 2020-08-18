
#include "shapefil.h"
#include "shp-common.h"
#include <memory.h>
#include <Rinternals.h>

SEXP shp_c_file_meta(SEXP path) {
  SHP_RESET_ERROR();

  const char* path0 = CHAR(STRING_ELT(path, 0));
  SEXP outRecords = PROTECT(Rf_allocVector(INTSXP, 1));
  SEXP outType = PROTECT(Rf_allocVector(INTSXP, 1));
  SEXP outBoundsMin = PROTECT(Rf_allocVector(REALSXP, 4));
  SEXP outBoundsMax = PROTECT(Rf_allocVector(REALSXP, 4));

  SHPHandle	hSHP = SHPOpen(path0, "rb");
  if (hSHP == NULL) {
    SHP_ERROR("%s", "SHPOpen: ");
  }

  SHPGetInfo(hSHP, INTEGER(outRecords), INTEGER(outType), REAL(outBoundsMin), REAL(outBoundsMax));

  SHPClose(hSHP);

  const char *names[] = {"shp_type", "n_features", "bounds_min", "bounds_max", ""};
  SEXP out = PROTECT(Rf_mkNamed(VECSXP, names));
  SET_VECTOR_ELT(out, 0, outType);
  SET_VECTOR_ELT(out, 1, outRecords);
  SET_VECTOR_ELT(out, 2, outBoundsMin);
  SET_VECTOR_ELT(out, 3, outBoundsMax);

  UNPROTECT(5);
  return out;
}

SEXP shp_c_geometry_meta(SEXP path, SEXP indices) {
  SHP_RESET_ERROR();

  int size = Rf_length(indices);
  int* pIndices = INTEGER(indices);

  // allocate out columns before interacting with the SHP api
  // as these allocations may fail
  SEXP shapeId = PROTECT(Rf_allocVector(INTSXP, size));
  SEXP nParts = PROTECT(Rf_allocVector(INTSXP, size));
  SEXP nVertices = PROTECT(Rf_allocVector(INTSXP, size));
  SEXP xMin = PROTECT(Rf_allocVector(REALSXP, size));
  SEXP yMin = PROTECT(Rf_allocVector(REALSXP, size));
  SEXP zMin = PROTECT(Rf_allocVector(REALSXP, size));
  SEXP mMin = PROTECT(Rf_allocVector(REALSXP, size));
  SEXP xMax = PROTECT(Rf_allocVector(REALSXP, size));
  SEXP yMax = PROTECT(Rf_allocVector(REALSXP, size));
  SEXP zMax = PROTECT(Rf_allocVector(REALSXP, size));
  SEXP mMax = PROTECT(Rf_allocVector(REALSXP, size));

  int* pShapeId = INTEGER(shapeId);
  int* pNparts = INTEGER(nParts);
  int* pNvertices = INTEGER(nVertices);
  double* pXMin = REAL(xMin);
  double* pYMin = REAL(yMin);
  double* pZMin = REAL(zMin);
  double* pMMin = REAL(mMin);
  double* pXMax = REAL(xMax);
  double* pYMax = REAL(yMax);
  double* pZMax = REAL(zMax);
  double* pMMax = REAL(mMax);

  const char* path0 = CHAR(STRING_ELT(path, 0));
  SHPHandle	hSHP = SHPOpen(path0, "rb");
  if (hSHP == NULL) {
    SHP_ERROR("%s", "SHPOpen: ");
  }

  // fast read mode skips a few steps that aren't needed
  // to extract number of parts, number of vertices, and
  // bounds for each feature
  SHPSetFastModeReadObject(hSHP, 1);
  int nFeatures = hSHP->nRecords;

  SHPObject* obj;
  for (int i = 0; i < size; i++) {
    // these are R-style 1-based indices
    if (pIndices[i] == NA_INTEGER || pIndices[i] > nFeatures) {
      pShapeId[i] = NA_INTEGER;
      pNparts[i] = NA_INTEGER;
      pNvertices[i] = NA_INTEGER;
      pXMin[i] = NA_REAL;
      pYMin[i] = NA_REAL;
      pZMin[i] = NA_REAL;
      pMMin[i] = NA_REAL;
      pXMax[i] = NA_REAL;
      pYMax[i] = NA_REAL;
      pZMax[i] = NA_REAL;
      pMMax[i] = NA_REAL;
      continue;
    }

    obj = SHPReadObject(hSHP, pIndices[i] - 1);
    if (obj == NULL) {
      SHPClose(hSHP);
      Rf_error("[i=%d] Error reading object for index %d", i + 1, pIndices[i]);
    }

    pShapeId[i] = obj->nShapeId;
    pNparts[i] = obj->nParts;
    pNvertices[i] = obj->nVertices;
    pXMin[i] = obj->dfXMin;
    pYMin[i] = obj->dfYMin;
    pZMin[i] = obj->dfZMin;
    pXMax[i] = obj->dfXMax;
    pYMax[i] = obj->dfYMax;
    pZMax[i] = obj->dfZMax;

    if (obj->bMeasureIsUsed) {
      pMMin[i] = obj->dfMMin;
      pMMax[i] = obj->dfMMax;
    } else {
      pMMin[i] = NA_REAL;
      pMMax[i] = NA_REAL;
    }

    SHPDestroyObject(obj);
  }

  SHPClose(hSHP);

  const char *names[] = {
    "shape_id", "n_parts", "n_vertices",
    "xmin", "ymin", "zmin", "mmin",
    "xmax", "ymax", "zmax", "mmax",
    ""
  };

  SEXP out = PROTECT(Rf_mkNamed(VECSXP, names));
  SET_VECTOR_ELT(out, 0, shapeId);
  SET_VECTOR_ELT(out, 1, nParts);
  SET_VECTOR_ELT(out, 2, nVertices);
  SET_VECTOR_ELT(out, 3, xMin);
  SET_VECTOR_ELT(out, 4, yMin);
  SET_VECTOR_ELT(out, 5, zMin);
  SET_VECTOR_ELT(out, 6, mMin);
  SET_VECTOR_ELT(out, 7, xMax);
  SET_VECTOR_ELT(out, 8, yMax);
  SET_VECTOR_ELT(out, 9, zMax);
  SET_VECTOR_ELT(out, 10, mMax);
  UNPROTECT(12);

  return out;
}
