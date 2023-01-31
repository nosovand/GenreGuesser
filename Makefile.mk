CC = g++ -std=c++17 -Ilibraries/ -Wall 

genreDetector: main.o data.o net.o
	$(CC) main.o data.o net.o -o $@

main.o: src/main.cpp libraries/net.hpp libraries/data.hpp
	$(CC)  -c src/main.cpp

data.o: src/data.cpp libraries/net.hpp libraries/data.hpp
	$(CC) -c src/data.cpp

net.o: src/net.cpp libraries/net.hpp libraries/data.hpp
	$(CC) -c src/net.cpp
