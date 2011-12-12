
PRESENTATION_INCLUDES = arch-01.svg arch-02.svg arch-03.svg proto.svg \
			system-full.svg system-working.svg kvm.svg rng.svg

PAPER_INCLUDES        = arch-03.svg proto.svg system-full.svg system-working.svg \
			kvm.svg rng.svg

INCLUDE_DIR = inc

BIBTEX    ?= bibtex -terse
LATEX     ?= pdflatex -file-line-error
LATEXFIRST  ?= -interaction=batchmode
LATEXLAST  ?=  -halt-on-error
INKSCAPE  ?= inkscape
CONVERT   ?= convert

export TEXINPUTS := .:./$(INCLUDE_DIR)/:$(TEXINPUTS)

PRESENTATION_INCLUDES := $(PRESENTATION_INCLUDES:%.svg=%.png)
PAPER_INCLUDES        := $(PAPER_INCLUDES:%.svg=%-g.png)

VPATH = inc

vpath %.png $(INCLUDE_DIR)
vpath %.svg $(INCLUDE_DIR)
vpath %.c   $(INCLUDE_DIR)

.PHONY: all clean allclean

all: presentation.pdf paper.pdf clean

clean:
	$(RM) *.aux *.bbl *.blg *.dvi *.log *.out *.nav *.snm *.toc *.vrb *~
	$(RM) $(INCLUDE_DIR)/*.png

allclean: clean
	$(RM) paper.pdf presentation.pdf

.PHONY: paper

paper: paper.pdf clean

paper.pdf: paper.aux paper.bbl
	$(LATEX) $(LATEXFIRST) paper.tex
	$(LATEX) $(LATEXLAST) paper.tex

paper.bbl: paper.aux refs.bib
	$(BIBTEX) -terse paper.aux

paper.aux: paper.tex $(PAPER_INCLUDES)
	$(LATEX)  $(LATEXFIRST) paper.tex

.PHONY: presentation

presentation: presentation.pdf clean

presentation.pdf: presentation.aux presentation.bbl
	$(LATEX)  $(LATEXFIRST) presentation.tex
	$(LATEX) $(LATEXLAST) presentation.tex

presentation.bbl: presentation.aux refs.bib
	$(BIBTEX) presentation.aux

presentation.aux: presentation.tex $(PRESENTATION_INCLUDES)
	$(LATEX)  $(LATEXFIRST) presentation.tex

# figures

%.png: %.svg
	$(INKSCAPE) -e $(INCLUDE_DIR)/$@ $<

%-g.png: %.png
	[[ -f $< ]] && prereq=$< || prereq=$(INCLUDE_DIR)/$<; \
	$(CONVERT) -colorspace gray $$prereq $(INCLUDE_DIR)/$@

