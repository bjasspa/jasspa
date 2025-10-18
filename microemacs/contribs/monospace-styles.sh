#!/bin/sh
########################################################
##
##  Copyright (c) 2025, MicroEmacs User
##  License       :
##  Date          : $Date$
##  Author        : $Author$
##  Created By    : MicroEmacs User
##  Created       : 2025-10-16 20:16:21
##  Last Modified : <251016.2024>
##
##  Description	  :
##
########################################################

fc-list :spacing=mono file | 
awk -F: '{print $1}' | 
while read -r fontfile; do     
  style=$(fc-query --format='%{style}\n' "$fontfile" 2>/dev/null);  
  family=$(fc-query --format='%{family}\n' "$fontfile" 2>/dev/null);     
      echo "$family: $style"; 
  done | sed -E 's/,[^:]+//' | sort | uniq | 
  awk -F: '{ n = split($2, arr, ","); 
      for (i = 1; i <= n; i++) print ($1," ",arr[i]) }' | sort | sed -E 's/  +/\t/' | uniq | less
