---
title: Sample Paper
author: Detlef Groth, University of Potsdam, Germany
date: 2025-03-27 08:38
include-before: |
    <style>
    body { 
      margin: 0 auto;
      max-width: 80%;
      padding-left: 50px;
      padding-right: 50px;
      padding-top: 50px;
      padding-bottom: 50px;
      hyphens: auto;
      overflow-wrap: break-word;
      font-family: Candara, sans-serif; 
    }
    h1 { text-align: center ;}
    p.author { text-align: center; }
    p.date { text-align: center; }
    pre  { font-family: Consolas; monospaced; background: #eee; padding: 10px;}
    table { border-spacing: 5px;  
           border-collapse: collapse;
           width: 200px; 
           border-bottom: 2px solid #333 ;      
           
    }
    th { 
        border-bottom: 2px solid #333 ;     
        border-top: 2px solid #333 ;             
        padding: 5px;
    }
    td { min-width: 30px; padding: 5px; }
    </style>
    <center>some links on top</center>
---

## Abstract

This is a sample paper abstract.
Which can spread out over several lines.
Lorem  ipsum  dolor sit amet,  consetetur  sadipscing  elitr, sed diam  nonumy
eirmod  tempor  invidunt  ut  labore et dolore  magna  aliquyatet  clita  kasd
gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum
dolor sit amet,  consetetur  sadipscing  elitr, sed diam nonumy  eirmod tempor
invidunt ut labore et dolore magna  aliquyat  vero eos et accusam et justo duo
dolores et ea rebum. Stet clita kasd  gubergren,  no sea takimata  sanctus est
Lorem ipsum dolor sit amet.

## Introduction

Lorem  ipsum  dolor sit amet,  consetetur  sadipscing  elitr, sed diam  nonumy
eirmod  tempor  invidunt  ut  labore et dolore  magna  aliquyatet  clita  kasd
gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum
dolor sit amet,  consetetur  sadipscing  elitr, sed diam nonumy  eirmod tempor
invidunt ut labore et dolore magna  aliquyat  vero eos et accusam et justo duo
dolores et ea rebum. Stet clita kasd  gubergren,  no sea takimata  sanctus est
Lorem ipsum dolor sit amet.

## Figures

```{r fig.cap="**Figure 1:** PCA of Iris data"}
data(iris)
plot(prcomp(iris[,1:4],scale=TRUE)$x[,1:2],col=as.numeric(iris$Species)+1,pch=15,cex=2)
grid()
legend('topright',fill=unique(as.numeric(iris$Species)+1),legend=unique(iris$Species))
```

## Tables

Below follows a table which is handwritten.


|     | col1 | col2 |
|:---:|:-----|:-----|
| 1   | A    | B    |
| 2   | C    | D    |
__Table 1:__ This is an example table


Now a table which is made by R:

```{r results="asis",echo=FALSE}
knitr::kable(head(iris))
```
__Table 2:__ This is a table generated with `knitr::kable`.

## Processing

THis document can be processed using a Bash script with the knitr package of R like this:

```
function rmd2html {
    if [ -z $2 ]; then
        echo "Usage: rmd2html infile.rmd outfile.html"
    else
        echo "library(knitr);knitr::knit('$1',output='temp.md')" | Rscript -
        pandoc -M document-css=false temp.md -o $2 -s
    fi
}
```




