OBJS=poly.o c00.o
HDRS=c0.h

c0: $(OBJS)
	$(CC) $(OBJS) -o c0

%.o: %.c $(HDRS)
	$(CC) -c $< -o $@

clean:
	rm -f *.o
	rm -f c0
