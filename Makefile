all:
	g++ -O3 -o yaforth yaforth.cpp middleware.cpp codegen.cpp

clean:
	rm yaforth

