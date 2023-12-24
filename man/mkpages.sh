#!/bin/sh

groff -t -e -mandoc -Tascii xfa.3 | col -bx > xfa.txt
groff -t -e -mandoc -Tps xfa.3 | ps2pdf - xfa.pdf
man2html < xfa.3 | sed 's/<BODY>/<BODY text="#0000FF" bgcolor="#FFFFFF" style="font-family: monospace;">/i' > xfa.html

