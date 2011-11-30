.PHONY: all clean allclean

all: paper

clean:
	$(RM) *.aux *.bbl *.blg *.dvi *.log *.out

allclean: clean
	$(RM) *.pdf

.PHONY: paper

paper: paper.pdf clean

paper.pdf: paper.aux paper.bbl
	pdflatex paper.tex
	pdflatex paper.tex

paper.bbl: paper.aux refs.bib
	bibtex paper.aux

paper.aux: paper.tex
	pdflatex paper.tex
