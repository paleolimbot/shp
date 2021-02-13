
#' List files associated with a shapefile
#'
#' @param file A .shp file
#' @param to Destination: a .shp filename the same length as `to` or
#'   a single directory.
#' @param ext One or more extensions to include
#' @param exists Use `FALSE` to generate hypothetical shapefile
#'   names.
#' @param overwrite Use `TRUE` to
#'
#' @return A list of files of length `length(file) * length(ext)`.
#' @export
#'
#' @examples
#' shp_assert(shp_example_all())
#' shp_list_files(shp_example("anno.shp"))
#'
#' dest <- tempfile()
#' dir.create(dest)
#'
#' shp_copy(shp_example("anno.shp"), dest)
#' list.files(dest)
#'
#' shp_move(file.path(dest, "anno.shp"), file.path(dest, "anno1.shp"))
#' list.files(dest)
#'
#' shp_delete(file.path(dest, "anno1.shp"))
#' list.files(dest)
#'
#' unlink(dest, recursive = TRUE)
#'
shp_list_files <- function(file, ext = shp_extensions(), exists = TRUE) {
  if (exists) {
    shp_assert(file)
  } else {
    stopifnot(all(endsWith(file, ".shp")))
  }

  file_grid <- vctrs::vec_rep_each(file, length(ext))
  ext_grid <- vctrs::vec_rep(ext, length(file))
  basename_out <- vapply(
    seq_along(file_grid),
    function(i) gsub("\\.shp$", paste0(".", ext_grid[i]), basename(file_grid[i])),
    character(1)
  )
  file_out <- file.path(dirname(file_grid), basename_out)

  if (exists) {
    file_out[file.exists(file_out)]
  } else {
    file_out
  }
}

#' @rdname shp_list_files
#' @export
shp_delete <- function(file, ext = shp_extensions()) {
  unlink(shp_list_files(file, ext = ext))
}

#' @rdname shp_list_files
#' @export
shp_copy <- function(file, to, ext = shp_extensions(), overwrite = FALSE) {
  shp_op2(file, to, "copy", ext = ext, overwrite = overwrite)
}

#' @rdname shp_list_files
#' @export
shp_move <- function(file, to, ext = shp_extensions(), overwrite = FALSE) {
  shp_op2(file, to, "move", ext = ext, overwrite = overwrite)
}

shp_op2 <- function(file, to, op, ext = shp_extensions(), overwrite = FALSE) {
  shp_assert(file)

  if ((length(to) == 1) && dir.exists(to)) {
    to <- file.path(to, basename(file))
  } else {
    stopifnot(all(endsWith(to, ".shp")))
    if (length(file) != length(to)) {
      stop("`file` must be the same length as `to`", call. = FALSE)
    }
  }

  for (i in seq_along(file)) {
    all_files <- shp_list_files(file[i], ext = ext)
    new_files <- shp_list_files(to[i], ext = tools::file_ext(all_files), exists = FALSE)

    # check if any of the ops are going to fail, since it's hard to
    # undo a partial copy/move
    if (op == "move" && any(file.exists(new_files))) {
      existing_files <- paste0("'", new_files[file.exists(new_files)], "'", collapse = ", ")
      stop(
        paste0("Can't overwrite existing files using shp_move():\n", existing_files),
        call. = FALSE
      )
    } else if (!overwrite && any(file.exists(new_files))) {
      existing_files <- paste0("'", new_files[file.exists(new_files)], "'", collapse = ", ")
      stop(
        paste0("Use `overwrite = TRUE` to overwrite existing files:\n", existing_files),
        call. = FALSE
      )
    }

    if (op == "move") {
      fs::file_move(all_files, new_files)
    } else {
      fs::file_copy(all_files, new_files, overwrite = overwrite)
    }
  }
}

#' @rdname shp_list_files
#' @export
shp_assert <- function(file) {
  problems <- list(
    "Is NA" = is.na(file),
    "Does not exist" = !is.na(file) & !file.exists(file),
    "Non-.shp extension" = !is.na(file) & !endsWith(file, ".shp"),
    "Missing .shx file" = !is.na(file) & !file.exists(gsub("\\.shp$", ".shx", file)),
    "Missing .dbf file" = !is.na(file) & !file.exists(gsub("\\.shp$", ".dbf", file))
  )

  if (any(Reduce("|", problems))) {
    count <- sum(Reduce("|", problems))
    files_lab <- if (count != 1) "files" else "file"

    problems <- problems[vapply(problems, any, logical(1))]
    problem_files <- vapply(problems, function(x) {
      bad_files <- file[x]
      if (length(bad_files) > 5) {
        paste0(
          paste0("'", bad_files[1:5], "'", collapse = ", "),
          "...and ", length(bad_files) - 5, " more."
        )
      } else {
        paste0("'", bad_files, "'", collapse = ", ")
      }
    }, character(1))

    stop(
      paste0(
        "Found ", count, " invalid .shp ", files_lab, ":\n",
        paste0(names(problems), ": ", problem_files, collapse = "\n")
      ),
      call. = FALSE
    )
  }

  invisible(file)
}

#' @rdname shp_list_files
#' @export
shp_extensions <- function() {
  # https://people.wou.edu/~taylors/es342/arc_gis_file_extensions.pdf
  c(
    "shp", "shx", "dbf", # mandatory: coords, index, dBase file
    "cpg", # code page (encoding for dbf)
    "prj", # projection
    "xml", "aux", # metadata
    "sbn", "sbx", "qix", # spatial index
    "fbn", "fbx", # read-only spatial index
    "atx", # ArcCatalog-related
    "ixs", "mxs" # geocoding indexeses
  )
}
