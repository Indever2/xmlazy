all:
	cd src && make compile

clean:
	cd src && make clean;
	rm -f priv/elixir_niftest.so