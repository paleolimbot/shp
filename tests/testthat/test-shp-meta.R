
test_that("shp_meta works", {
  expect_identical(nrow(shp_meta(character(0))), 0L)
  expect_identical(nrow(shp_meta(shp_example("mexico/cities.shp"))), 1L)
  expect_identical(nrow(shp_meta(shp_example(rep("mexico/cities.shp", 2)))), 2L)

  expect_identical(nrow(shp_meta(shp_example_all())), length(shp_example_all()))

  expect_error(shp_meta("does_not_exist.shp"), "Unable to open")
})

test_that("shp_geometry works", {
  cities <- shp_geometry_meta(shp_example("mexico/cities.shp"))
  expect_is(cities, "data.frame")
  expect_identical(nrow(cities), shp_meta(shp_example("mexico/cities.shp"))$n_features)
  expect_identical(cities$xmin, cities$xmax)
  expect_identical(cities$ymin, cities$ymax)

  nas <- shp_geometry_meta(shp_example("mexico/cities.shp"), indices = c(1, 36, NA, 37))
  expect_identical(nas$shape_id, c(0L, 35L, NA, NA))

  all_metas <- shp_meta(shp_example_all())
  all_geometry_metas <- lapply(shp_example_all(), shp_geometry_meta)
  expect_identical(vapply(all_geometry_metas, nrow, integer(1)), all_metas$n_features)

  expect_error(shp_geometry_meta(shp_example("mexico/cities.shp"), indices = 0), "Error reading obj")
  expect_error(shp_geometry_meta("does_not_exist.shp"), "Unable to open")
  expect_error(shp_geometry_meta("does_not_exist.shp", indices = 1), "Unable to open")
})
