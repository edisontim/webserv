CXX			= clang++

CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -pedantic

SRCS		= webserv.cpp Internet_socket.cpp

OBJS		= $(SRCS:.cpp=.o)

NAME		= webserv

RM			= rm -rf



all		:	$(NAME)

$(NAME)	:	$(OBJS)
				$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

run		:	$(NAME)
				./$(NAME)

clean	:	
				$(RM) $(OBJS)

fclean	:	clean
				$(RM) $(NAME)

re		:	fclean all

.PHONY	:	all fclean re
