
SRC		= 	./src/ReseauGTFS.cpp	\
			./src/graphe.cpp		\
			./src/main.cpp

CXX		= g++

CXXFLAGS	= -W -Wall -Wextra -std=c++11

INCDIR		= 
RM		= rm -f
LIBPATH = -L ./src/
DATALIB = -l TP1

DBGFLAG		= -g3

OBJ		= $(SRC:.cpp=.o)

NAME		= test_exe

ECHO		= @/bin/echo -e

all:		$(NAME)

$(NAME):	$(OBJ)
		$(CXX) -o $(NAME) $(OBJ) $(LIBPATH) $(DATALIB)
		$(ECHO) "\n\033[1mBuild successful.\033[0m\n"

debug:		$(OBJ)
		$(CXX) -o $(NAME) $(DBGFLAG) $(OBJ) $(LIBPATH) $(DATALIB)
		$(ECHO) "\n\033[1mDebug Build successful.\033[0m\n"

clean:
		$(RM) $(OBJ)
		$(ECHO) "\n\033[1mObject files deleted.\033[0m\n"

fclean:		clean
		$(RM) $(NAME)
		$(ECHO) "\n\033[1mBinary file deleted.\033[0m\n"

re:			fclean all
.PHONY:		all clean fclean re debug