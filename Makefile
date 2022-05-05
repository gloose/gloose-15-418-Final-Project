APP_NAME=Board
OBJS += Board.o
OBJS += Move.o
OBJS += Piece.o
OBJS += Position.o

CXX = mpic++ -std=c++11
CXXFLAGS = -I. -O3 -g #-Wall -Wextra

default: $(APP_NAME)

$(APP_NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	/bin/rm -rf *~ *.o $(APP_NAME) *.class
