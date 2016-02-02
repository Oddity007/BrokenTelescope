#blah
all:
	#c++ -shared -fPIC -std=c++11 -Weverything -O0 -I /usr/local/include -L /usr/local/lib -llua -g -I . -lsfml-graphics -lsfml-window -lsfml-system -fvisibility=default *.cpp -o generated_dependencies/libImageGenerator.dylib
	c++ -std=c++11 -Weverything -O0 -I /usr/local/include -L /usr/local/lib -llua -g -I . -lsfml-graphics -lsfml-window -lsfml-system -lfreetype -fvisibility=default *.cpp -o generated_dependencies/gen


test:
	#lua main.lua
	./generated_dependencies/gen

testbot:
	lua main.lua

clean:
	rm -rf generated_dependencies/*
