
test_that("read_dbf_meta() works", {
  all_dbf <- list.files(
    system.file("shp", package = "shp"), ".dbf",
    recursive = TRUE,
    full.names = TRUE
  )

  for (dbf in all_dbf) {
    expect_is(read_dbf_meta(!! dbf), "data.frame")
  }
})
