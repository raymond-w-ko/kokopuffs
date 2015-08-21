all:
	clang++ \
		-Wall -std=c++11 -stdlib=libc++ -lc++abi \
		-O0 -g3 -fstack-protector-all -fsanitize=address \
		-o test main.cpp
	./test
