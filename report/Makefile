.PHONY: all clean allclean

all: paper presentation

clean:
	$(RM) *.aux *.bbl *.blg *.dvi *.log *.out *.nav *.snm *.toc *~

allclean: clean
	$(RM) paper.pdf presentation.pdf

.PHONY: paper

paper: paper.pdf clean

paper.pdf: paper.aux paper.bbl
	pdflatex paper.tex
	pdflatex paper.tex

paper.bbl: paper.aux refs.bib
	bibtex paper.aux

paper.aux: paper.tex
	pdflatex paper.tex

.PHONY: presentation

presentation: presentation.pdf clean

presentation.pdf: presentation.aux presentation.bbl
	pdflatex presentation.tex
	pdflatex presentation.tex

presentation.bbl: presentation.aux refs.bib
	bibtex presentation.aux

presentation.aux: presentation.tex
	pdflatex presentation.tex