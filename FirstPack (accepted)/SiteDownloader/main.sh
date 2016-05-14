#!/bin/bash

count=0

site_down() {
	count=$(( $count+1 ))
	dir=$count'_dir'
	mkdir $dir
	cd $dir
	GET $1 > "page_$count.html"
	GET $1 > "text_$count.txt"

	grep -E -o 'https?:[^"]+' text_$count.txt | while read line; do echo $line >> result_$count.txt; done

	cat result_$count.txt | while read line
	do
		site_down $line
	done
}

site_down $1
