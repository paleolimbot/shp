
test_that("shapelib_version works", {
  expect_identical(shapelib_version(), package_version("1.5.0"))
})
