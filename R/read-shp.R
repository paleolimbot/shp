
#' Read .shp files
#'
#' @inheritParams shp_meta
#' @inheritParams read_dbf
#' @param geometry_col The column name in which
#'
#' @return A [tibble::tibble()] with geometry column
#'  created using [shp_geometry()].
#' @export
#'
#' @examples
#' read_shp(shp_example("mexico/cities.shp"))
#'
#' @importFrom rlang :=
read_shp <- function(file, col_spec = "?", encoding = NA, geometry_col = "geometry") {
  shp_assert(file)
  result <- read_dbf(file, col_spec = col_spec, encoding = encoding)
  vctrs::vec_cbind(result, !! geometry_col := shp_geometry(file))
}
