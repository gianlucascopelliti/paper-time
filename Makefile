
all: paper.pdf

paper.pdf: .FORCE
	$(MAKE) -C figures
	latexmk -pdf paper.tex

.FORCE:

clean:

distclean: clean
	rm -f paper.pdf

