CXX			= clang++

CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -pedantic

SRCS_PATH	= srcs

SRCS		= webserv.cpp Internet_socket.cpp Server.cpp utils.cpp

SRCS_NAME	= $(addprefix $(SRCS_PATH)/, $(SRCS))

OBJS		= $(SRCS_NAME:.cpp=.o)

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
