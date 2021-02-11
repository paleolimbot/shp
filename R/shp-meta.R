
#' Read shapefile meta information
#'
#' @param file A vector of filenames.
#' @param indices A vector of positive integers giving the
#'   indices of the desired features.
#'
#' @return A [tibble::tibble()]
#' @export
#'
#' @examples
#' shp_meta(shp_example("mexico/cities.shp"))
#' shp_geometry_meta(shp_example("mexico/cities.shp"))
#'
shp_meta <- function(file) {
  if (length(file) != 1) {
    metas <- lapply(file, shp_meta)
    ptype <- tibble::new_tibble(
      list(
        file = character(0),
        shp_type = character(0),
        n_features = integer(0),
        xmin = double(), ymin = double(), zmin = double(), mmin = double(),
        xmax = double(), ymax = double(), zmax = double(), mmax = double()
      ),
      nrow = 0L
    )
    return(vctrs::vec_rbind(ptype, !!! metas))
  }

  file_exp <- path.expand(file)
  meta <- .Call(shp_c_file_meta, file_exp)

  # make ranges into their own columns
  ranges <- c(as.list(meta$bounds_min), as.list(meta$bounds_max))
  names(ranges) <- c("xmin", "ymin", "zmin", "mmin", "xmax", "ymax", "zmax", "mmax")
  meta$bounds_min <- NULL
  meta$bounds_max <- NULL

  # make shp_type human-readable
  meta$shp_type <- shp_types$shp_type[match(meta$shp_type, shp_types$shp_type_id)]

  tibble::new_tibble(c(list(file = file), meta, ranges), nrow = 1L)
}

#' @rdname shp_meta
#' @export
shp_geometry_meta <- function(file, indices = NULL) {
  file <- path.expand(file)
  stopifnot(length(file) == 1)

  if (is.null(indices)) {
    indices <- seq_len(shp_meta(file)$n_features)
  } else {
    indices <- as.integer(indices)
  }

  result <- .Call(shp_c_geometry_meta, file, indices)
  tibble::new_tibble(result, nrow = length(result[[1]]))
}

shp_types <- list(
  shp_type_id = c(0, 1, 3, 5, 8, 11, 13, 15, 18, 21, 23, 25, 28, 31),
  shp_type = c(
    "null", "point", "arc", "polygon", "multipoint",
    "pointz", "arcz", "polygonz", "multipointz", "pointm",
    "arcm", "polygonm", "multipointm", "multipatch"
  )
)
