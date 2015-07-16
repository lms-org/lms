#!/bin/sh

# For xsltproc install package xsltproc
# For xmllint install package libxml-utils

if [ -z $1 ]; then
  echo "Usage: validate-xml.sh <file.xml>"
  exit 1
fi

xsltproc preprocess.xsl "$1" | xmllint --noout --schema framework.xsd -
