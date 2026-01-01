OBJS=poly.o globals.o getnum.o c00.o c01.o c02.o c03.o c04.o main.o
HDRS=c0.h

c0: $(OBJS) c0.h
	$(CC) $(OBJS) -o c0

%.o: %.c $(HDRS)
	$(CC) -c $< -o $@

clean:
	rm -f *.o
	rm -f c0
