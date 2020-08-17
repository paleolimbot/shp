
test_that("shp_meta works", {
  expect_identical(nrow(shp_meta(character(0))), 0L)
  expect_identical(nrow(shp_meta(shp_example("mexico/cities.shp"))), 1L)
  expect_identical(nrow(shp_meta(shp_example(rep("mexico/cities.shp", 2)))), 2L)

  expect_identical(nrow(shp_meta(shp_example_all())), length(shp_example_all()))

  expect_error(shp_meta("does_not_exist.shp"), "Unable to open")
})
