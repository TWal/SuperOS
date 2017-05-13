.PHONY: all clean

all: report.pdf

report.pdf: report.tex report.bib Makefile
	pdflatex report.tex
	bibtex report
	pdflatex report.tex
	pdflatex report.tex

clean:
	rm -rf `find . | grep -v "^.$$" | grep -v "report.tex" | grep -v "report.bib" | grep -v "Makefile"`
