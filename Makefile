pong_clone: main.cc
	g++ main.cc -lsfml-graphics -lsfml-window -lsfml-system -o pong_clone

run: pong_clone
	./pong_clone

.PHONY: clean
clean:
	rm -f pong_clone