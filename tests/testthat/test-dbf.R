
test_that("read_dbf() works", {
  dbf <- shp_example("mexico/cities.dbf")
  expect_true(all(vapply(read_dbf(dbf, "c"), is.character, logical(1))))
  expect_true(all(vapply(read_dbf(dbf, "cccc"), is.character, logical(1))))

  expect_equal(ncol(read_dbf(dbf, col_spec = "-")), 0)
  expect_equal(nrow(read_dbf(dbf, col_spec = "-")), 36)

  dbf_chr <- read_dbf(dbf, "c")
  dbf_auto <- read_dbf(dbf)
  expect_is(dbf_auto$POPULATION, "numeric")
  expect_identical(as.numeric(dbf_chr$POPULATION), dbf_auto$POPULATION)

  # only 'F' field in the examples
  expect_is(read_dbf(shp_example("pline.dbf"))$LENGTH, "numeric")

  expect_error(read_dbf(dbf, "cc"), "Can't use")
  expect_error(read_dbf(dbf, NA), "Expected string vector")
  expect_error(read_dbf(dbf, c("c", "c")), "Expected string vector")
})

test_that("read_dbf() reports parse errors", {
  expect_warning(
    read_dbf(shp_example("csah.dbf"), col_spec = "????????l"),
    "Found 58 parse problems"
  )
  expect_silent(read_dbf(shp_example("csah.dbf"), col_spec = "?") )
})

test_that("read_dbf() can accept a file encoding", {
  expect_identical(
    read_dbf(shp_example("eccities.shp"))$label[3],
    "Québec"
  )

  expect_identical(
    read_dbf(shp_example("eccities.shp"), encoding = "UTF-8")$label[3],
    "Québec"
  )

  expect_identical(
    read_dbf(shp_example("eccities.shp"), encoding = "windows-1252")$label[3],
    "QuÃ©bec"
  )
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
