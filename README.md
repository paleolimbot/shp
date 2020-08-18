
<!-- README.md is generated from README.Rmd. Please edit that file -->

# shp

<!-- badges: start -->

[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://www.tidyverse.org/lifecycle/#experimental)
<!-- badges: end -->

The goal of shp is to provide low-level access to ESRI shapefile
metadata, attributes, and geometry. It uses the excellent
[shapelib](http://shapelib.maptools.org/) C library.

## Installation

You can install the development version from
[GitHub](https://github.com/) with:

``` r
# install.packages("remotes")
remotes::install_github("paleolimbot/shp")
```

## Example

This is a basic example which shows you how to solve a common problem:

``` r
library(shp)
shp_file_meta(shp_example("mexico/cities.shp"))
#> # A tibble: 1 x 11
#>   path       shp_type n_features  xmin  ymin  zmin  mmin  xmax  ymax  zmax  mmax
#>   <chr>      <chr>         <int> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl>
#> 1 /Library/… point            36 -115.  16.6     0     0 -88.3  32.6     0     0
shp_geometry_meta(shp_example("mexico/cities.shp"))
#> # A tibble: 36 x 11
#>    shape_id n_parts n_vertices   xmin  ymin  zmin  mmin   xmax  ymax  zmax  mmax
#>       <int>   <int>      <int>  <dbl> <dbl> <dbl> <dbl>  <dbl> <dbl> <dbl> <dbl>
#>  1        0       0          1 -100.   25.7     0    NA -100.   25.7     0    NA
#>  2        1       0          1 -106.   23.2     0    NA -106.   23.2     0    NA
#>  3        2       0          1 -103.   20.7     0    NA -103.   20.7     0    NA
#>  4        3       0          1  -97.8  22.2     0    NA  -97.8  22.2     0    NA
#>  5        4       0          1  -99.1  19.4     0    NA  -99.1  19.4     0    NA
#>  6        5       0          1  -98.2  19.0     0    NA  -98.2  19.0     0    NA
#>  7        6       0          1  -96.1  19.0     0    NA  -96.1  19.0     0    NA
#>  8        7       0          1  -97.0  16.9     0    NA  -97.0  16.9     0    NA
#>  9        8       0          1  -89.6  20.8     0    NA  -89.6  20.8     0    NA
#> 10        9       0          1 -115.   32.6     0    NA -115.   32.6     0    NA
#> # … with 26 more rows
```
