
test_that("read_shx() works", {
  shx <- read_shx(shp_example("eccities.shp"))
  expect_true(all(shx$content_length == 10))
  expect_true(all(diff(shx$offset) == 14))

  # check randomized indices, as these are sorted internally
  random_order <- c(4L, 2L, 5L, 1L, 3L, 6L)
  expect_identical(
    read_shx(shp_example("eccities.shp"), indices = random_order),
    read_shx(shp_example("eccities.shp"))[random_order, ]
  )

  expect_error(read_shx("not a file", indices = 1), "Failed to open shx")
})

test_that("shx_meta() works", {
  expect_identical(
    shx_meta(character()),
    tibble::tibble(file = character(), n_features = integer())
  )

  expect_identical(shx_meta(shp_example("eccities.shp"))$n_features, 6L)

  expect_error(shx_meta("not a file"), "Failed to open shx")
})
