
test_that("read_shx() works", {
  shx <- read_shx(shp_example("eccities.shp"))
  expect_true(all(shx$content_length == 10))
  expect_true(all(diff(shx$offset) == 14))
})

test_that("shx_meta() works", {
  expect_identical(
    shx_meta(character()),
    tibble::tibble(file = character(), n_features = integer())
  )

  expect_identical(shx_meta(shp_example("eccities.shp"))$n_features, 6L)
})
