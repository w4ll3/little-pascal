NAME = pascal

CXXFLAGS = -std=c++11

OBJS = parser.o tokenizer.o ast.o utils.o main.o ccalc.o

all: $(NAME)

parser.cpp: ${NAME}.y
	bison -d -o parser.cpp ${NAME}.y

parser.hpp: parser.cpp

tokenizer.cpp: ${NAME}.l
	flex -o tokenizer.cpp ${NAME}.l

%.o: %.cpp ast.h utils.h ccalc.h
	g++ -c $(CXXFLAGS) -g -o $@ $< 


$(NAME): $(OBJS)
	g++ -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

# make debug - check the bison output report
debug:
	bison -d -o parser.cpp ${NAME}.y -v

clean:
	@rm -f parser.output *.o parser.cpp parser.hpp tokenizer.cpp
