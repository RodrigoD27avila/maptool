LIB = libmaptool.a
OBJS=maptool.o maperror.o

all: $(LIB)

.c.o:
	$(CC) -c $< -ggdb -DMAP_DEBUG

$(LIB): $(OBJS)
	$(AR) rcs $(LIB) $(OBJS)

clean:
	rm *.o libmaptool.a
