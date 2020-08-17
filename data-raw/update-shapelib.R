
library(tidyverse)

src_url <- "http://download.osgeo.org/shapelib/shapelib-1.5.0.tar.gz"
curl::curl_download(src_url, "data-raw/shapelib.tar.gz")
untar("data-raw/shapelib.tar.gz", exdir = "data-raw")
src_dir <- list.files("data-raw", pattern = "shapelib-", full.names = TRUE)
stopifnot(length(src_dir) == 1)

src_files <- list.files(src_dir, "\\.(c|h)$", full.names = TRUE)
file.copy(src_files, file.path("src", basename(src_files)))

