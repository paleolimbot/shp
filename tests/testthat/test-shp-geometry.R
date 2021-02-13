
test_that("shp_geometry print/format works", {
  all_shp <- shp_example_all()
  for (shp in all_shp) {
    meta <- shp_meta(shp)
    expect_identical(
      expect_output(print(shp_geometry(!! shp)), meta$shp_type),
      shp_geometry(!! shp)
    )
  }

  expect_identical(
    expect_output(
      print(new_shp_geometry(integer(), shp_example("anno.shp"))),
      "\\[0\\]"
    ),
    new_shp_geometry(integer(), shp_example("anno.shp"))
  )
})

test_that("shp_geometry print/format does not error for invalid file", {
  expect_output(print(new_shp_geometry(integer(), "not a shapefile")), "INVALID\\[0\\]")
  expect_message(
    expect_output(
      print(new_shp_geometry(1L, "not a shapefile")), "INVALID\\[1\\]"
    ),
    "Does not exist"
  )
})
