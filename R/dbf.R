
read_dbf <- function(file) {
  cpp_read_dbf(path.expand(file))
}

read_dbf_meta <- function(file) {
  result <- cpp_read_dbf_meta(path.expand(file))
  result$type <- rawToChar(result$type, multiple = TRUE)
  new_data_frame(result)
}
