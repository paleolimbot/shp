
#' Read shapefile meta information
#'
#' @param path A vector of filenames.
#' @param indices A vector of positive integers giving the
#'   indices of the desired features.
#'
#' @return A `list()`
#' @export
#'
#' @examples
#' shp_file_meta(shp_example("mexico/cities.shp"))
#'
shp_file_meta <- function(path) {
  if (length(path) == 0) {
    new_data_frame(
      list(
        path = character(0),
        shp_type = character(0),
        n_features = integer(0),
        xmin = double(), ymin = double(), zmin = double(), mmin = double(),
        xmax = double(), ymax = double(), zmax = double(), mmax = double()
      ),
      nrow = 0L
    )
  } else if (length(path) > 1) {
    metas <- lapply(path, shp_file_meta)
    do.call(rbind, metas)
  } else {
    path_exp <- path.expand(path)
    meta <- .Call(shp_c_file_meta, path_exp)

    # make ranges into their own columns
    ranges <- c(as.list(meta$bounds_min), as.list(meta$bounds_max))
    names(ranges) <- c("xmin", "ymin", "zmin", "mmin", "xmax", "ymax", "zmax", "mmax")
    meta$bounds_min <- NULL
    meta$bounds_max <- NULL

    # make shp_type human-readable
    meta$shp_type <- shp_types$shp_type[match(meta$shp_type, shp_types$shp_type_id)]

    new_data_frame(c(list(path = path), meta, ranges), nrow = 1L)
  }
}

#' @rdname shp_file_meta
#' @export
shp_geometry_meta <- function(path, indices = NULL) {
  path <- path.expand(path)
  stopifnot(length(path) == 1)

  if (is.null(indices)) {
    indices <- seq_len(shp_file_meta(path)$n_features)
  } else {
    indices <- as.integer(indices)
  }

  new_data_frame(.Call(shp_c_geometry_meta, path, indices))
}

shp_types <- list(
  shp_type_id = c(0, 1, 3, 5, 8, 11, 13, 15, 18, 21, 23, 25, 28, 31),
  shp_type = c(
    "null", "point", "arc", "polygon", "multipoint",
    "pointz", "arcz", "polygonz", "multipointz", "pointm",
    "arcm", "polygonm", "multipointm", "multipatch"
  )
)
