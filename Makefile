all:
	g++ -o yaforth yaforth.cpp middleware.cpp codegen.cpp

clean:
	rm yaforth

