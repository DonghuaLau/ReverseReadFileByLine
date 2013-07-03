all:
	g++ Demo.cpp -o demo -lReverseReadFileByLine
so:
	g++ ReverseReadFileByLine.cpp -fPIC -shared -o libReverseReadFileByLine.so

