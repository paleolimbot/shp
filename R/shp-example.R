
#' Read an example shapefile
#'
#' @param example_path An example filename (see examples for a list of names)
#'
#' @return A full path to the example file
#' @export
#'
#' @examples
#' # all example names
#' list.files(system.file("shp", package = "shp"), "\\.shp", recursive = TRUE)
#'
#' # get full path of one example file
#' shp_example("mexico/cities.shp")
#'
#' # get full path of all example files
#' shp_example_all()
#'
shp_example <- function(example_path) {
  file.path(system.file("shp", package = "shp"), example_path)
}

#' @rdname shp_example
#' @export
shp_example_all <- function() {
  list.files(system.file("shp", package = "shp"), "\\.shp", recursive = TRUE, full.names = TRUE)
}
