---
title: Examples for undesired wrapping in MicroEmacs
author: Detlef Groth
date: 2025-12-11 13:59
---

## Examples

To check the example you need to have set the _$fill-column_ variable to _78_.

### Code between backticks

__Unwrapped:__

In this  paragraph  the text within  single back ticks (graves)  should be not
separated during automatic  wrapping as this in some cases is might break code
as line based  parsers  might need that the opening and the closing  grave are
not in the same line - here some  examples  `nfig  label`  `nfig  label` `nfig label` `nfig label` `nfig label` `nfig label.

If you wrap the  paragraph  above one of the code  examples is broken into two
lines.

__Wrapped:__

In this  paragraph  the text within  single back ticks (graves)  should be not
separated during automatic  wrapping as this in some cases is might break code
as line based  parsers  might need that the opening and the closing  grave are
not in the same line - here some  examples  `nfig  label`  `nfig  label` `nfig
label` `nfig label` `nfig label` `nfig label.


### Link breaking and ugly spaces at the beginning of a list

The first example  breaks fine, as there are no spaces in the link, the second
is not so nice as it has too many spaces:

__Unwrapped:__

- _Left:_ GUI version - theme "Solarized Light", displaying on top a help page defined using Markdown (bottom window) for the [pydoc](contribs/hkpydoc.emf) macro 
- _Right:_ GUI version - of the Ayu themes, [Ayu Dark](https://github.com/ayu-theme), displaying the hypertext enabled R documentation browser defined with the r-doc command


__Wrapped:__

- _Left:_ GUI version - theme "Solarized Light", displaying on top a help page
  defined using Markdown (bottom window) for the [pydoc](contribs/hkpydoc.emf)
  macro 
- _Right:_     GUI     version     -    of    the     Ayu     themes,     [Ayu
  Dark](https://github.com/ayu-theme),  displaying  the  hypertext  enabled  R
  documentation browser defined with the r-doc command

## Code chunks

There  should be no wrapping in code chunks.  Please your cursor at the end of
the first chunk line. And enter a space, that then ugly wraps the code.

```{r}
data = read.table("../data-2023/20230123-1k-combined_Otu_Tax.csv",header=TRUE,sep=",")
season=rep("Wet",nrow(data))
season[data$month %in% c("January","February","March","April","November","December")]="Dry"
```

That is the wrapped code.

```{r}
data                                                                         =
read.table("../data-2023/20230123-1k-combined_Otu_Tax.csv",header=TRUE,sep=",") 
season=rep("Wet",nrow(data))
season[data$month                                                         %in%
c("January","February","March","April","November","December")]="Dry" 
```



