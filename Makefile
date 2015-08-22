		# -fsanitize=address \

all:
	clang++ \
		-DDEBUG \
		-Wall -std=c++11 -stdlib=libc++ -lc++abi \
		-O0 -g3 -fstack-protector-all \
		-o test main.cpp 2>&1
	valgrind ./test 2>&1
