% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/read-shp.R
\name{read_shp}
\alias{read_shp}
\title{Read .shp files}
\usage{
read_shp(file, col_spec = "?", encoding = NA, geometry_col = "geometry")
}
\arguments{
\item{file}{A vector of filenames.}

\item{col_spec}{A character vector of length one with
one character for each column or one character to be used
for all columns. The following characters are supported
(designed to align with \code{\link[readr:cols]{readr::cols()}}):
\itemize{
\item "?": Use DBF-specified field type
\item "-": Skip column
\item "c": Character
\item "i": Parse integer
\item "d": Parse double
\item "l": Parse as logical
}}

\item{encoding}{Use \code{NA} to automatically guess encoding,
\code{""} to use system encoding, or a length-one character
vector overriding the automatically detected encoding.}

\item{geometry_col}{The column name in which}
}
\value{
A \code{\link[tibble:tibble]{tibble::tibble()}} with geometry column
created using \code{\link[=shp_geometry]{shp_geometry()}}.
}
\description{
Read .shp files
}
\examples{
read_shp(shp_example("mexico/cities.shp"))

}
