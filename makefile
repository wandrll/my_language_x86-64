CFLAGS= -c -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=null -fsanitize=object-size -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr 
LDFLAGS= -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=null -fsanitize=object-size -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr
TMP = -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=null -fsanitize=object-size -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr
CTEMP=-Wall -Werror -Wextra -pedantic -Wshadow -Wconversion -Wsign-conversion
Lexer_dir=binary_translator
Cpu_dir=cpu
Obj_dir=obj
all: compiler




compiler: $(Lexer_dir)/main.o  $(Lexer_dir)/build_tree.o $(Lexer_dir)/lexer.o $(Lexer_dir)/table_name.o $(Lexer_dir)/tree.o  $(Lexer_dir)/translator.o $(Lexer_dir)/tree.h 
	g++ $(LDFLAGS) $(Lexer_dir)/main.o  $(Lexer_dir)/build_tree.o $(Lexer_dir)/lexer.o $(Lexer_dir)/table_name.o $(Lexer_dir)/tree.o  $(Lexer_dir)/translator.o -o comp 

main.o: $(Lexer_dir)/main.cpp $(Lexer_dir)/table_name.h $(Lexer_dir)/tree.h
	g++ $(CFLAGS) $(Lexer_dir)/main.cpp

build_tree.o: $(Lexer_dir)/build_tree.cpp $(Lexer_dir)/tree.h
	g++ $(CFLAGS) $(Lexer_dir)/build_tree.cpp 

lexer.o: $(Lexer_dir)/lexer.cpp $(Lexer_dir)/tree.h
	g++ $(CFLAGS) $(Lexer_dir)/lexer.cpp

table_name.o: $(Lexer_dir)/table_name.cpp $(Lexer_dir)/table_name.h
	g++ $(CFLAGS) $(Lexer_dir)/table_name.cpp

tree.o: $(Lexer_dir)/tree.cpp $(Lexer_dir)/tree.h
	g++ $(CFLAGS) $(Lexer_dir)/tree.cpp

translator.o: $(Lexer_dir)/translator.cpp $(Lexer_dir)/tree.h
	g++ $(CFLAGS) $(Lexer_dir)/translator.cpp


clean:
	rm -rf *.o comp stack/*.o  $(Lexer_dir)/*.o