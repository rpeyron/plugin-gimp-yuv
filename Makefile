GIMPTOOL=gimptool-2.0
CFLAGS = -Wall -g -O3 `gimptool --cflags`
VERSION=0.1.3
DIR="yuv-$(VERSION)"

FILES= \
	yuv.c \
	Makefile \
	Makefile.win \
	README \
	yuv.dev

all: yuv

yuv: yuv.c
	$(GIMPTOOL) --build yuv.c

install: yuv
	$(GIMPTOOL) --install-bin yuv

uninstall:
	$(GIMPTOOL) --uninstall yuv
    
dist: $(FILES)
	mkdir $(DIR)
	cp $(FILES) $(DIR)
	tar cvzf "$(DIR).tar.gz" $(DIR)
	rm -Rf $(DIR)

clean:
	rm -f yuv
