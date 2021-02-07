
#' Read .dbf files
#'
#' @param path A filename of a .dbf file.
#' @param col_spec A character vector of length one with
#'   one character for each column. Use "c" to return the
#'   character representation.
#'
#' @return A data.frame
#' @export
#'
#' @examples
#' dbf <- system.file("shp/mexico/cities.dbf", package = "shp")
#' read_dbf(dbf)
#' read_dbf_meta(dbf)
#'
read_dbf <- function(path, col_spec = "") {
  result <- cpp_read_dbf(path.expand(path), col_spec)
  new_data_frame(
    result[!vapply(result, is.null, logical(1))],
    nrow = attr(result, "row.names")
  )
}

#' @rdname read_dbf
#' @export
read_dbf_meta <- function(path) {
  result <- cpp_read_dbf_meta(path.expand(path))
  result$type <- rawToChar(result$type, multiple = TRUE)
  new_data_frame(result)
}
