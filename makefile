proj4:
	echo "" > out.csv; \
	echo "" > out2.csv; \
	g++ proj4.cpp -o proj4 -lm -fopenmp
	g++ -DENDYEAR=2116 proj4.cpp -o longrun -lm -fopenmp
	./proj4 >>out.csv ; \
	./longrun >>out2.csv
clean:
	rm -f *.o
