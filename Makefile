		# -fsanitize=address \

all:
	clang++ \
		-Wall -std=c++11 -stdlib=libc++ -lc++abi \
		-O3 -march=native \
		-lboost_system -lboost_chrono \
		-o test main.cpp 2>&1
	./test 2>&1

debug:
	g++ \
		-DDEBUG \
		-Wall -std=c++11 \
		-O0 -g3 -fstack-protector-all \
		-lboost_system -lboost_chrono \
		-o test main.cpp 2>&1
	valgrind --leak-check=full ./test 2>&1
