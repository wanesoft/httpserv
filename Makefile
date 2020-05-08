# **************************************************************************************************** #
#                                                                                                      #
#                                                          :::::::::   ::::::::   ::::::::      :::    #
#   Makefile                                              :+:    :+: :+:    :+: :+:    :+:   :+: :+:   #
#                                                        +:+    +:+ +:+    +:+ +:+         +:+   +:+   #
#   By: Ivan Marochkin <i.marochkin@rosalinux.ru>       +#++:++#:  +#+    +:+ +#++:++#++ +#++:++#++:   #
#                                                      +#+    +#+ +#+    +#+        +#+ +#+     +#+    #
#   Created: 2019/10/03 10:17:25 by Ivan Marochkin    #+#    #+# #+#    #+# #+#    #+# #+#     #+#     #
#   Updated: 2019/10/03 10:17:25 by Ivan Marochkin   ###    ###  ########   ########  ###     ###      #
#                                                                                                      #
# **************************************************************************************************** #

OBJDIR		=	obj
SRCDIR		=	src
OBJ			=	$(addprefix $(OBJDIR)/, $(patsubst $(SRCDIR)/%.cpp, %.o, $(wildcard $(SRCDIR)/*.cpp)))
HDR			=	$(SRCDIR)/*.hpp
TARGET		=	httpserv
GCC			=	g++
CPPFLAGS	=	-std=c++11 -lpthread -lssl -lcrypto -Wall -Wextra

.PHONY: all clean

all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(GCC) $(CPPFLAGS) -c $< -o $@

$(TARGET): $(OBJ) $(HDR)
	$(GCC) $(CPPFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(wildcard *.o)
	rm -rf $(OBJDIR)

re: clean all