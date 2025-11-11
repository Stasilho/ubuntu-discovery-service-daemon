TARGET_EXE = ./out/serviceLauncher
CXX = g++
CXXFLAGS = -std=c++20

PATH_SRC = ./src/
PATH_OBJ = ./obj/

SRC = $(wildcard $(PATH_SRC)*.cpp)
OBJ = $(patsubst $(PATH_SRC)%.cpp, $(PATH_OBJ)%.o, $(SRC))

$(TARGET_EXE) : $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET_EXE)

$(PATH_OBJ)%.o : $(PATH_SRC)%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	rm $(TARGET_EXE) $(PATH_OBJ)*.o