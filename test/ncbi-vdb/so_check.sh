#!/bin/bash

K=$(nm $1 | grep " U " | grep -v GLIBC | grep -v " U xml");
if [ "$K" != "" ];
	then echo "Error: $1 contains unresolved external references:\n $K" && exit 1;
fi