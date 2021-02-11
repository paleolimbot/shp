
#' Read .dbf files
#'
#' @param path A filename of a .dbf file.
#' @param col_spec A character vector of length one with
#'   one character for each column or one character to be used
#'   for all columns. The following characters are supported
#'   (designed to align with [readr::cols()]):
#'
#'   - "?": Use DBF-specified field type
#'   - "-": Skip column
#'   - "c": Character
#'   - "i": Parse integer
#'   - "d": Parse double
#'   - "l": Parse as logical
#'
#' @param encoding Override the automatically guessed
#'   encoding.
#'
#' @return A [tibble::tibble()]
#' @export
#'
#' @examples
#' read_dbf(shp_example("mexico/cities.dbf"))
#' dbf_colmeta(shp_example("mexico/cities.dbf"))
#'
read_dbf <- function(path, col_spec = "?", encoding = "") {
  result <- cpp_read_dbf(path.expand(path), col_spec, encoding)

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
dbf_meta <- function(path) {
  if (length(path) > 1) {
    metas <- lapply(path, dbf_meta)
    return(do.call(rbind, metas))
  }

  result <- cpp_dbf_meta(path.expand(path))
  tibble::new_tibble(c(list(path = path), result), nrow = length(result[[1]]))
}

#' @rdname read_dbf
#' @export
dbf_colmeta <- function(path) {
  result <- cpp_dbf_colmeta(path.expand(path))
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
