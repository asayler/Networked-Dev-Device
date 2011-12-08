
PRESENTATION_INCLUDES = arch-01.svg arch-02.svg arch-03.svg system-full.svg system-working.svg
PAPER_INCLUDES =

INC_PATH = inc/
PRESENTATION_INCLUDES := $(addprefix $(INC_PATH), $(PRESENTATION_INCLUDES:%.svg=%.png))
PAPER_INCLUDES        := $(addprefix $(INC_PATH), $(PAPER_INCLUDES:%.svg=%.png))
export TEXINPUTS := .:./$(INC_PATH)/:$(TEXINPUTS)

.PHONY: all clean allclean

all: paper presentation

clean:
	$(RM) *.aux *.bbl *.blg *.dvi *.log *.out *.nav *.snm *.toc *~
	$(RM) inc/*.png

allclean: clean
	$(RM) paper.pdf presentation.pdf

.PHONY: paper

paper: paper.pdf clean

paper.pdf: paper.aux paper.bbl
	pdflatex paper.tex
	pdflatex paper.tex

paper.bbl: paper.aux refs.bib
	bibtex paper.aux

paper.aux: paper.tex $(PAPER_INCLUDES)
	pdflatex paper.tex

.PHONY: presentation

presentation: presentation.pdf clean

presentation.pdf: presentation.aux presentation.bbl
	pdflatex presentation.tex
	pdflatex presentation.tex

presentation.bbl: presentation.aux refs.bib
	bibtex presentation.aux

presentation.aux: presentation.tex $(PRESENTATION_INCLUDES)
	pdflatex presentation.tex

# figures

%.png: %.svg
	inkscape -e $@ $<
