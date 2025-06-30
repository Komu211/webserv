VPATH		=	$(SRC_DIR):$(SRC_DIR)/server:$(SRC_DIR)/config

SRCS		=	main.cpp \
				Server.cpp \
				GlobalConfig.cpp \
				ServerConfig.cpp \
				LocationConfig.cpp \
				Socket.cpp \
				ActiveSockets.cpp \


INCLUDES	=	-Iincludes \
				-Iincludes/server \
				-Iincludes/config