CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -Wconversion -O0 -g
PROGS = tipos bits desborde regs_leer leds_regs boton_regs sensores panel

.PHONY: all clean
all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

regs_leer leds_regs boton_regs panel: gpio_regs.h

clean:
	rm -f $(PROGS) boton_regs panel desborde_ub b_con_O0 b_sin_O0 b_con_O2 b_sin_O2 con.s sin.s *.o
