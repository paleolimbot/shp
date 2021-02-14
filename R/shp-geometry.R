
#' Create a shapefile geometry vector
#'
#' @inheritParams shp_meta
#' @param x A vector of zero-based indices corresponding to the
#'   internal `shape_id` within the shapefile.
#' @inheritParams wk::wk_crs
#'
#' @return A vector
#' @export
#'
#' @examples
#' shp_geom <- shp_geometry(shp_example("3dpoints.shp"))
#' shp_geom
#'
#' wk::as_wkb(shp_geom)
#' wk::as_xy(shp_geom)
#'
shp_geometry <- function(file) {
  shp_assert(file)

  file <- fs::path_abs(path.expand(file))
  n_features <- .Call(shp_c_file_meta, file)$n_features

  if (n_features == 0) {
    new_shp_geometry(integer(), file = file)
  } else {
    new_shp_geometry(seq(0L, n_features - 1L), file = file)
  }
}

#' @importFrom wk wk_handle
#' @export
wk_handle.shp_geometry <- function(handleable, handler, ...) {
  .Call(shp_c_handle_geometry, handleable, wk::as_wk_handler(handler))
}

#' @importFrom wk wk_crs
#' @export
wk_crs.shp_geometry <- function(x) {
  attr(x, "crs")
}

#' @rdname shp_geometry
#' @export
new_shp_geometry <- function(x, file, crs = NULL) {
  vctrs::vec_assert(x, integer())
  vctrs::new_vctr(x, file = file, crs = crs, class = "shp_geometry")
}

#' @importFrom vctrs vec_ptype_abbr
#' @export
vec_ptype_abbr.shp_geometry <- function(x, ...) {
  tryCatch({
    meta <- shp_meta(attr(x, "file"))
    sprintf("shp:%s", meta$shp_type)
  }, error = function(e) {
    "shp:INVALID"
  })
}

#' @export
print.shp_geometry <- function(x, ...) {
  cat(sprintf("<%s[%d]>\n", vec_ptype_abbr(x), length(x)))
  cat(sprintf("%s: %s\n", attr(x, "file"), shp_index_summary(x)))
  tryCatch(
    shp_assert(attr(x, "file")),
    error = function(e) message(paste(e, collapse = "\n"))
  )

  if (length(x) == 0) {
    return(invisible(x))
  }

  print(format(x, ...), quote = FALSE)
  invisible(x)
}

#' @export
format.shp_geometry <- function(x, ...) {
  meta <- try(shp_meta(attr(x, "file")), silent = TRUE)
  if (inherits(meta, "try-error")) {
    return(sprintf("INVALID {%s}", vctrs::vec_data(x)))
  }

  geometry_meta <- shp_geometry_meta(attr(x, "file"), indices = vctrs::vec_data(x) + 1L)
  has_m <- any(is.finite(geometry_meta$mmin))

  switch(
    meta$shp_type,
    "null" = rep("-", length(x)), # nocov
    "point" = sprintf(
      "(%s %s)",
      format(geometry_meta$xmin, ...),
      format(geometry_meta$ymin, ...)
    ),
    "pointm" = sprintf(
      "(%s %s %s)",
      format(geometry_meta$xmin, ...),
      format(geometry_meta$ymin, ...),
      format(geometry_meta$mmin, ...)
    ),
    "pointz" = if (has_m) {
      sprintf(
        "(%s %s %s %s)",
        format(geometry_meta$xmin, ...),
        format(geometry_meta$ymin, ...),
        format(geometry_meta$zmin, ...),
        format(geometry_meta$mmin, ...)
      )
    } else {
      sprintf(
        "(%s %s %s)",
        format(geometry_meta$xmin, ...),
        format(geometry_meta$ymin, ...),
        format(geometry_meta$zmin, ...)
      )
    },
    sprintf(
      "[%s %s...%s %s]",
      format(geometry_meta$xmin, ...), format(geometry_meta$ymin, ...),
      format(geometry_meta$xmax, ...), format(geometry_meta$ymax, ...)
    )
  )
}

# For RStudio View()
#' @export
as.character.shp_geometry <- function(x, ...) {
  format(x)
}

shp_index_summary <- function(x) {
  x <- vctrs::vec_data(x)
  if (length(x) < 7) {
    sprintf("{%s}", paste(x, collapse = ", "))
  } else {
    sprintf("{%s, ..., %s}", paste(x[1:5], collapse = ", "), x[length(x)])
  }
}
