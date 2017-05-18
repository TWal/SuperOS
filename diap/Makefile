.PHONY: all

all:
	pdflatex diapo.tex
	mkdir -p diapodir
	convert -depth 24 -density 203.2 diapo.pdf diapodir/_.bmp
