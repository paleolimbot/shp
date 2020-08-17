#' @keywords internal
"_PACKAGE"

# The following block is used by usethis to automatically manage
# roxygen namespace tags. Modify with care!
## usethis namespace: start
#' @useDynLib shp, .registration = TRUE
## usethis namespace: end
NULL

#' External software versions
#'
#' @export
#'
#' @examples
#' shapelib_version()
#'
shapelib_version <- function() {
  package_version(.Call(shp_c_shapelib_version))
}
