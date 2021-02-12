
test_that("shp_list_files() works", {
  expect_setequal(
    basename(shp_list_files(shp_example("anno.shp"))),
    c("anno.shp", "anno.shx", "anno.dbf", "anno.aux")
  )

  expect_setequal(
    dirname(shp_list_files(shp_example_all())),
    c(shp_example("mexico"), gsub("/$", "", shp_example("")))
  )
})

test_that("shp_copy() works", {
  dest <- tempfile()
  dir.create(dest)

  expect_setequal(list.files(dest), character())
  shp_copy(shp_example(c("anno.shp", "3dpoints.shp")), dest)
  expect_setequal(
    list.files(dest),
    c("3dpoints.dbf", "3dpoints.shp", "3dpoints.shx", "anno.aux", "anno.dbf", "anno.shp", "anno.shx")
  )
  expect_true(all(file.exists(shp_example(c("anno.shp", "3dpoints.shp")))))
  expect_error(
    shp_copy(shp_example(c("anno.shp", "3dpoints.shp")), dest),
    "overwrite existing files"
  )

  expect_error(
    shp_copy(
      shp_example(c("anno.shp", "3dpoints.shp")),
      file.path(dest, c("anno.shp", "3dpoints.shp", "csah.shp"))
    ),
    "must be the same length"
  )

  shp_copy(
    shp_example(c("anno.shp", "3dpoints.shp", "csah.shp")),
    file.path(dest, c("anno.shp", "3dpoints.shp", "csah.shp")),
    overwrite = TRUE
  )

  expect_setequal(
    list.files(dest),
    c(
      "3dpoints.dbf", "3dpoints.shp", "3dpoints.shx",
      "anno.aux", "anno.dbf", "anno.shp", "anno.shx",
      "csah.dbf", "csah.shp", "csah.shx"
    )
  )

  unlink(dest, recursive = TRUE)
})

test_that("shp_move() works", {
  dest <- tempfile()
  dir.create(dest)

  shp_copy(shp_example("anno.shp"), dest)
  expect_setequal(
    list.files(dest),
    c("anno.aux", "anno.dbf", "anno.shp", "anno.shx")
  )

  expect_error(shp_move(file.path(dest, "anno.shp"), dest), "Can't overwrite existing")

  shp_move(file.path(dest, "anno.shp"), file.path(dest, "anno1.shp"))
  expect_setequal(
    list.files(dest),
    c("anno1.aux", "anno1.dbf", "anno1.shp", "anno1.shx")
  )

  unlink(dest, recursive = TRUE)
})

test_that("shp_delete() works", {
  dest <- tempfile()
  dir.create(dest)

  shp_copy(shp_example("anno.shp"), dest)
  expect_setequal(
    list.files(dest),
    c("anno.aux", "anno.dbf", "anno.shp", "anno.shx")
  )

  shp_delete(file.path(dest, "anno.shp"))
  expect_setequal(list.files(dest), character())

  unlink(dest, recursive = TRUE)
})

test_that("shp_assert() works", {
  expect_identical(shp_assert(shp_example_all()), shp_example_all())
  expect_error(shp_assert(paste0(shp_example_all(), "x")), "Found 18 invalid .shp files")
  expect_error(shp_assert(NA_character_), "Found 1 invalid .shp file")
})
