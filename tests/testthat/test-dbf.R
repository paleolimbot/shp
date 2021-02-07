
test_that("read_dbf() works", {
  dbf <- system.file("shp/mexico/cities.dbf", package = "shp")
  expect_true(all(vapply(read_dbf(dbf, "c"), is.character, logical(1))))
  expect_true(all(vapply(read_dbf(dbf, "cccc"), is.character, logical(1))))

  expect_equal(ncol(read_dbf(dbf, col_spec = "-")), 0)
  expect_equal(nrow(read_dbf(dbf, col_spec = "-")), 36)

  dbf_chr <- read_dbf(dbf, "c")
  dbf_auto <- read_dbf(dbf, "")
  expect_is(dbf_auto$POPULATION, "numeric")
  # works because precision == 0 for this field
  expect_identical(as.numeric(dbf_chr$POPULATION), dbf_auto$POPULATION)

  expect_error(read_dbf(dbf, "cc"), "Can't use")
  expect_error(read_dbf(dbf, NA), "Expected string vector")
  expect_error(read_dbf(dbf, c("c", "c")), "Expected string vector")
})

test_that("read_dbf() runs for all example dbf files", {
  all_dbf <- list.files(
    system.file("shp", package = "shp"), ".dbf",
    recursive = TRUE,
    full.names = TRUE
  )

  for (dbf in all_dbf) {
    expect_is(read_dbf(!! dbf), "data.frame")
  }
})

test_that("read_dbf_meta() runs for all example dbf files", {
  all_dbf <- list.files(
    system.file("shp", package = "shp"), ".dbf",
    recursive = TRUE,
    full.names = TRUE
  )

  for (dbf in all_dbf) {
    expect_is(read_dbf_meta(!! dbf), "data.frame")
  }
})
