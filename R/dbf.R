
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
#' read_dbf(shp_example("mexico/cities.dbf"))
#' read_dbf_meta(shp_example("mexico/cities.dbf"))
#'
read_dbf <- function(path, col_spec = "?") {
  result <- cpp_read_dbf(path.expand(path), col_spec)

  df <- tibble::new_tibble(
    result[!vapply(result, is.null, logical(1))],
    nrow = attr(result, "n_rows")
  )

  problems <- attr(result, "problems")
  if (length(problems[[1]]) > 0) {
    problems$file <- path
    attr(df, "problems") <- tibble::new_tibble(problems, nrow = length(problems[[1]]))
  }

  warn_problems(df)
  df
}

#' @rdname read_dbf
#' @export
read_dbf_meta <- function(path) {
  result <- cpp_read_dbf_meta(path.expand(path))
  result$type <- rawToChar(result$type, multiple = TRUE)
  tibble::new_tibble(result, nrow = length(result[[1]]))
}

warn_problems <- function(df) {
  problems <- attr(df, "problems")
  if (!is.null(problems)) {
    warning(
      sprintf(
        "Found %s parse %s. See attr(, \"problems\") for details.",
        nrow(problems),
        if (nrow(problems) != 1) "problems" else "problem"
      ),
      call. = FALSE, immediate. = TRUE
    )
  }

  invisible(df)
}
