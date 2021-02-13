
test_that("read_shp() works", {
  cities <- read_shp(shp_example("mexico/cities.shp"))
  expect_is(cities$geometry, "shp_geometry")
  expect_identical(cities[-5], read_dbf(shp_example("mexico/cities.dbf")))

  expect_message(
    read_shp(shp_example("mexico/cities.shp"), geometry_col = "POPULATION"),
    "New names:"
  )
})
