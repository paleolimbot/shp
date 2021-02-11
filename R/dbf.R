
#' Read .dbf files
#'
#' @param file A filename of a .dbf file.
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
#' @param encoding Use `NA` to automatically guess encoding,
#'   `""` to use system encoding, or a length-one character
#'   vector overriding the automatically detected encoding.
#'
#' @return A [tibble::tibble()]
#' @export
#'
#' @examples
#' read_dbf(shp_example("mexico/cities.dbf"))
#' dbf_meta(shp_example("mexico/cities.dbf"))
#' dbf_colmeta(shp_example("mexico/cities.dbf"))
#'
read_dbf <- function(file, col_spec = "?", encoding = NA) {
  # encoding of "" typically means "system" in R, but for simpifying the
  # C++ code, we use "" to mean "Unknown" in C++.
  if (identical(encoding, NA)) {
    encoding <- ""
  } else if (identical(encoding, "")) {
    encoding <- gsub("^[^.]+\\.", "", Sys.getlocale("LC_COLLATE"))
  }

  result <- cpp_read_dbf(path.expand(file), col_spec, encoding)

  df <- tibble::new_tibble(
    result[!vapply(result, is.null, logical(1))],
    nrow = attr(result, "n_rows")
  )

  problems <- attr(result, "problems")
  if (length(problems[[1]]) > 0) {
    problems$file <- file
    attr(df, "problems") <- tibble::new_tibble(problems, nrow = length(problems[[1]]))
  }

  warn_problems(df)
  df
}

#' @rdname read_dbf
#' @export
dbf_meta <- function(file) {
  if (length(file) != 1) {
    metas <- lapply(file, dbf_meta)
    ptype <- tibble::tibble(
      file = character(),
      row_count = integer(),
      field_count = integer(),
      encoding = character()
    )

    return(vctrs::vec_rbind(ptype, !!! metas))
  }

  result <- cpp_dbf_meta(path.expand(file))

  # convert C++ encoding ("" == Unknown) to more intuitive R form
  # consistent with input of read_dbf()
  if (identical(result$encoding, "")) {
    result$encoding <- NA_character_
  }

  tibble::new_tibble(c(list(file = file), result), nrow = length(result[[1]]))
}

#' @rdname read_dbf
#' @export
dbf_colmeta <- function(file) {
  result <- cpp_dbf_colmeta(path.expand(file))
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
