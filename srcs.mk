VPATH		=	$(SRC_DIR):$(SRC_DIR)/server:$(SRC_DIR)/config:$(SRC_DIR)/utils:$(SRC_DIR)/request:$(SRC_DIR)/request/types:$(SRC_DIR)/request/response:$(SRC_DIR)/request/cgi

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
				HTTPRequestData.cpp \
                GETRequest.cpp \
                DELETERequest.cpp \
                POSTRequest.cpp \
                ErrorRequest.cpp \
                ResponseWriter.cpp \
                CGISubprocess.cpp


INCLUDES	=	-Iincludes \
				-Iincludes/server \
				-Iincludes/config \
				-Iincludes/utils \
				-Iincludes/request \
				-Iincludes/request/types \
				-Iincludes/request/response \
				-Iincludes/request/cgi
