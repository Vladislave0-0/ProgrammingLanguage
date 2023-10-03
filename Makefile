CFLAGS = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations   \
         -Wmissing-include-dirs -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -pipe -fexceptions -Wcast-qual	                        \
         -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op                   \
         -Wmissing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192           \
         -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -D_DEBUG -D_EJUDGE_CLIENT_SIDE -fsanitize=address -g

TARGET = lang

CC = g++

all:
	@$(CC) $(CFLAGS) main.cpp ./frontend/InputProcessing.cpp ./frontend/Tree.cpp ./frontend/RecursiveDescent.cpp ./graphviz/GraphvizFunctions.cpp -o $(TARGET)
    
clean:
	rm $(TARGET)
