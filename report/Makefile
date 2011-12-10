
PRESENTATION_INCLUDES = arch-01.svg arch-02.svg arch-03.svg proto.svg \
			system-full.svg system-working.svg kvm.svg rng.svg

PAPER_INCLUDES        = arch-03.svg proto.svg

INC_PATH = inc/
PRESENTATION_INCLUDES := $(addprefix $(INC_PATH), $(PRESENTATION_INCLUDES:%.svg=%.png))
PAPER_INCLUDES        := $(addprefix $(INC_PATH), $(PAPER_INCLUDES:%.svg=%.png))
export TEXINPUTS := .:./$(INC_PATH)/:$(TEXINPUTS)

BIBTEX    ?= bibtex -terse
LATEX     ?= pdflatex -file-line-error
INKSCAPE  ?= inkscape

.PHONY: all clean allclean

all: paper presentation

clean:
	$(RM) *.aux *.bbl *.blg *.dvi *.log *.out *.nav *.snm *.toc *.vrb *~
	$(RM) inc/*.png

allclean: clean
	$(RM) paper.pdf presentation.pdf

.PHONY: paper

paper: paper.pdf clean

paper.pdf: paper.aux paper.bbl
	$(LATEX) paper.tex
	$(LATEX) paper.tex

paper.bbl: paper.aux refs.bib
	$(BIBTEX) -terse paper.aux

paper.aux: paper.tex $(PAPER_INCLUDES)
	$(LATEX) paper.tex

.PHONY: presentation

presentation: presentation.pdf clean

presentation.pdf: presentation.aux presentation.bbl
	$(LATEX) presentation.tex
	$(LATEX) presentation.tex

presentation.bbl: presentation.aux refs.bib
	$(BIBTEX) presentation.aux

presentation.aux: presentation.tex $(PRESENTATION_INCLUDES)
	$(LATEX) presentation.tex

# figures

%.png: %.svg
	$(INKSCAPE) -e $@ $<
