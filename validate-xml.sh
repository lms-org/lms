#!/bin/sh

# For xsltproc install package xsltproc
# For xmllint install package libxml2-utils

if [ -z "$1" ]; then
  echo "Usage: validate-xml.sh <file.xml>"
  exit 1
fi

validate() {
  xsltproc preprocess.xsl "$1" | xmllint --noout --schema framework.xsd -
}

# green
highlightColor="\033[0;32m"
highlightReset="\033[0m"

if [ -d "$1" ]; then
  for file in $(find "$1" -type f -iname '*.xml' -printf '%P\n'); do
    echo "${highlightColor}${file}${highlightReset}"
    validate "$1/$file"
  done
else
  validate "$1"
fi
