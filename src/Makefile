CFLAGS=-fpic -shared -dynamiclib -undefined dynamic_lookup
INC=/usr/local/lib/erlang/usr/include/
OBJECTS=elixir_niftest.o chars.o items.o parse.o

compile: $(OBJECTS)
	CC -o elixir_niftest.so $(OBJECTS) $(CFLAGS) -I $(INC);
	@if [ ! -d obj ]; then mkdir obj; fi;
	@rm -rf obj/*;
	@mv *.o obj/ && cp elixir_niftest.so obj/ && mv elixir_niftest.so ../priv/ 

elixir_niftest.o: elixir_niftest.c
	CC -c elixir_niftest.c -I $(INC)

chars.o: chars.c
	CC -g -O -c chars.c -I $(INC)

items.o: items.c
	CC -g -O -c items.c -I $(INC)

parse.o: parse.c
	CC -g -O -c parse.c -I $(INC)

clean:
	rm -f *.o;
	rm -rf obj/*