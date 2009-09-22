all:
	cd thirdparty && $(MAKE)
	cd src && $(MAKE)
	cp src/image.sq .
	python ./util/sq2hex.py < image.sq > image.hex
	python ./util/sq2js.py < image.sq > jsubleq/image.js

run: all
	#./thirdparty/sqrun image.sq && echo
	./src/sq image.sq && echo

clean:
	cd thirdparty && $(MAKE) clean
	cd src && $(MAKE) clean
	rm image.sq image.hex

realclean:
	rm jsubleq/image.js
