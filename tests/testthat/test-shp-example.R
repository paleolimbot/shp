
test_that("shp_example works", {
  all_shp <- list.files(system.file("shp", package = "shp"), ".shp", recursive = TRUE)
  expect_true(all(file.exists(shp_example(all_shp))))
})
