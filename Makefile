PODDIR=/usr/lib/perl-5.005/bin
PUBLISHDIR=/usr/local/lan/htdocs/util/sqar

all: sqar.html sqar.man

publish: sqar.html sqar.man sqar
	cp $? $(PUBLISHDIR)

sqar.html: sqar
	$(PODDIR)/pod2html $? > $@

sqar.man: sqar
	$(PODDIR)/pod2man $? > $@
