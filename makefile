OBJS = simulator.cpp simulator_functions.cpp
NAME = simulator
CPP_FLAGS = -Wall -l ncurses

all: $(NAME)

$(NAME): $(OBJS)
	g++ $(OBJS) -o $(NAME) $(CPP_FLAGS)

debug: $(OBJS)
	g++ $(OBJS) -o $(NAME) $(CPP_FLAGS) -g

clean:
	rm -rf $(NAME) 

# Our headers.
HEADERS = *.h
