#!/usr/bin/env Rscript

x <- "ä"
if ("ä" == "ä") {
    print("This code contains Umlauts äöüÄÖÜ and the sz: ß!")
} else {
    print("This is not running iso-8859-15!")
}
