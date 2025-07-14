#!/usr/bin/env Rscript
options(encoding = "ISO-8859-15")
library(RSQLite)
hello <- function(argv) {
    print("Hello R World! We support Umlauts ... äöüßÖÄÜ")
}
show_args <- function(argv) {
    print(paste("args where:", argv, collapse = ", "))
}
main <- function(argv) {
    hello()
    print("main called")
    print(argv)
    con <- dbConnect(RSQLite::SQLite(), dbname = ":memory:")
    fields <- dbListTables(con)
    dbDisconnect(con)
}
if (sys.nframe() == 0L && !interactive()) {
    binname <- gsub("--file=", "", grep("--file", commandArgs(), value = TRUE)[1])
    main(c(binname, commandArgs(trailingOnly = TRUE)))
}

