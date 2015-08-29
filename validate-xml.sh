#!/bin/sh

# For xsltproc install package xsltproc
# For xmllint install package libxml2-utils

if [ -z "$1" ]; then
  echo "Usage: validate-xml.sh <file.xml>"
  exit 1
fi

# check if stdout is a terminal...
if [ -t 1 ]; then
  # see if it supports colors...
  ncolors=$(tput colors)

  if test -n "$ncolors" && test ${ncolors} -ge 8; then
    normal="$(tput sgr0)"
    green="$(tput setaf 2)"
  fi
fi

validate() {
  xsltproc preprocess.xsl "$1" | xmllint --noout --schema framework.xsd -
}

if [ -d "$1" ]; then
  for file in $(find "$1" -type f -iname '*.xml' -printf '%P\n'); do
    echo "${green}${file}${normal}"
    validate "$1/$file"
  done
else
  validate "$1"
fi
