#!/bin/bash

site_down () {

	mkdir $1_dir
	cd $1_dir
	GET $1 > "page_$1_.html"
	GET $1 > "text_$1_.txt"

	grep -E -o '"https?:[^"]+"' text_$1_.txt | while read line; do echo "$line" >> result_$1.txt; done
	
	cat result_$1.txt | while read line
	do
 		site_down $line
	done
}

site_down $1


