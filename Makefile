		# -fsanitize=address \

all:
	clang++ \
		-DDEBUG \
		-Wall -std=c++11 -stdlib=libc++ -lc++abi \
		-O0 -g3 -fstack-protector-all \
		-o test main.cpp
	valgrind ./test 2>&1
