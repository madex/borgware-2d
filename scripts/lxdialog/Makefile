HOSTCFLAGS += -DLOCALE 
LIBS = -lncurses

ifeq (/usr/include/ncurses/ncurses.h, $(wildcard /usr/include/ncurses/ncurses.h))
        HOSTCFLAGS += -I/usr/include/ncurses -DCURSES_LOC="<ncurses.h>"
else
ifeq (/usr/pkg/include/ncurses/ncurses.h, $(wildcard /usr/pkg/include/ncurses/ncurses.h))
        HOSTCFLAGS += -I/usr/pkg/include -I/usr/pkg/include/ncurses -DCURSES_LOC="<ncurses.h>" -L/usr/pkg/lib -Wl,-R/usr/pkg/lib
else
ifeq (/usr/include/ncurses/curses.h, $(wildcard /usr/include/ncurses/curses.h))
        HOSTCFLAGS += -I/usr/include/ncurses -DCURSES_LOC="<ncurses/curses.h>"
else
ifeq (/usr/include/ncurses.h, $(wildcard /usr/include/ncurses.h))
        HOSTCFLAGS += -DCURSES_LOC="<ncurses.h>"
else
	HOSTCFLAGS += -DCURSES_LOC="<curses.h>"
endif
endif
endif
endif


OBJS = checklist.o menubox.o textbox.o yesno.o inputbox.o \
       util.o lxdialog.o msgbox.o

%.o: %.c
	$(HOSTCC) $(HOSTCFLAGS) -c -o $@ $<

all: ncurses lxdialog

lxdialog: $(OBJS)
	$(HOSTCC) $(HOSTCFLAGS) -o lxdialog $(OBJS) $(LIBS)

ncurses:
	@echo "int main() {return 0;}" > lxtemp.c
	@if $(HOSTCC) $(HOSTCFLAGS) -lncurses lxtemp.c ; then \
		rm -f lxtemp.c a.out; \
	else \
		rm -f lxtemp.c; \
		echo -e "\007" ;\
		echo ">> Unable to find the Ncurses libraries." ;\
		echo ">>" ;\
		echo ">> You must have Ncurses installed in order" ;\
		echo ">> to use 'make menuconfig'" ;\
		echo ;\
		exit 1 ;\
	fi

clean:
	rm -f core *.o *~ lxdialog
