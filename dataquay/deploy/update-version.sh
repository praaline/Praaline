#!/bin/bash

usage() {
    echo "Usage: $0 <version>"
    echo "  e.g. $0 2.2"
    exit 2;
}

version=$1
[ -n "$version" ] || usage

major=${version%%.*} # 2.3 -> 2, 2.3.1 -> 2
minor=${version#*.}  # 2.3 -> 3, 2.3.1 -> 3.1
point=${minor#*.}    # 3 -> 3, 3.1 -> 1
if [ "$minor" = "$point" ]; then point=0; fi
minor=${minor%.*}    # 3 -> 3, 3.1 -> 3

echo "Major version = $major, minor version = $minor, point release = $point"

acs="`echo $version | tr '.' '_'`"
echo "acsymbols string = $acs"

p="perl -i -p -e"

$p 's/(This is version) [^ ]+/$1 '$version'/' \
    README.txt

$p 's/(VERSION)=[0-9\.]+/$1='$version'/' \
    lib.pro

$p 's/(Version:) .*/$1 '$version'/' \
    deploy/dataquay.pc.in

if ! grep -q "$acs" src/acsymbols.c ; then
    $p 's/^$/\nextern void dataquay_v_'$acs'_present(void) { }/' \
	src/acsymbols.c
fi

