
#' Query a .shx shapefile index
#'
#' @param file A .shx file
#' @param indices A vector of indices or NULL to get all offsets.
#'
#' @return A [tibble::tibble()] with columns `offset` and `content_length`.
#' @export
#'
#' @examples
#' shx_meta(shp_example("3dpoints.shp"))
#' read_shx(shp_example("3dpoints.shp"))
#'
read_shx <- function(file, indices = NULL) {
  file <- make_shx(file)

  if (is.null(indices)) {
    n_records <- (file.size(file) - 100L) %/% 8
    if (n_records == 0) {
      indices <- integer()
    } else {
      indices <- seq(0L, n_records - 1L)
    }

    index_order <- NULL
  } else {
    index_order <- order(indices)
    indices <- as.integer(indices) - 1L
    indices <- indices[index_order]
  }

  result <- .Call(shp_c_read_shx, path.expand(file), indices)

  if (!is.null(index_order)) {
    result <- lapply(result, "[", order(index_order))
  }

  tibble::new_tibble(result, nrow = length(result[[1]]))
}

#' @rdname read_shx
#' @export
shx_meta <- function(file) {
  file <- make_shx(file)

  if (length(file) != 1) {
    metas <- lapply(file, shx_meta)
    ptype <- tibble::tibble(
      file = character(),
      n_features = integer()
    )

    return(vctrs::vec_rbind(ptype, !!! metas))
  }

  result <- .Call(shp_c_shx_meta, path.expand(file))
  tibble::new_tibble(c(list(file = file), result), nrow = length(result[[1]]))
}

# allow .shp files here also!
make_shx <- function(file) {
  gsub("\\.shp$", ".shx", file)
}
