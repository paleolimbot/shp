
new_data_frame <- function(x, nrow = length(x[[1]])) {
  structure(x, row.names = c(NA, -as.integer(nrow)), class = "data.frame")
}
