VPATH		=	$(SRC_DIR):$(SRC_DIR)/server:$(SRC_DIR)/config:$(SRC_DIR)/utils:$(SRC_DIR)/request:$(SRC_DIR)/request/types:$(SRC_DIR)/request/response

SRCS		=	main.cpp \
				Server.cpp \
				GlobalConfig.cpp \
				ServerConfig.cpp \
				LocationConfig.cpp \
				Socket.cpp \
				PollManager.cpp \
				utils.cpp \
				HTTPRequestFactory.cpp \
                HTTPRequestParser.cpp \
				HTTPRequest.cpp \
                GETRequest.cpp \
                DELETERequest.cpp \
                POSTRequest.cpp \
                ErrorRequest.cpp \
                ResponseWriter.cpp


INCLUDES	=	-Iincludes \
				-Iincludes/server \
				-Iincludes/config \
				-Iincludes/utils \
				-Iincludes/request \
				-Iincludes/request/types \
				-Iincludes/request/response
