CC              = g++
#########################
# local shared library
#########################
DLIB_DIR_H      = /var/www/shared
DLIB_DIR        = $(DLIB_DIR_H)/dlibs
DLIB_DIR_LPATH  = $(foreach dir,$(DLIB_DIR),   -L$(dir)) # add prefix to all dir
DLIB_DIR_H_IPATH= $(foreach dir,$(DLIB_DIR_H), -I$(dir)) # add prefix to all dir
DLIB_DIR_RPATH  = $(foreach dir,$(DLIB_DIR),   -Wl,-rpath=$(dir)) # add prefix to all dir

#########################
# my shared library
#########################
DLIB_NAME       = -ltoken -ldatabasepp -lutilpp#-lerror -lfileUtil -lmemoryManager -lstackTracer -larrayList_noSync -lmap_ArrayList_noSync -labstractFactoryCommon -lcweb -lcwebpp -lutilpp #-ldatabasepp#-lclientOutput_strMap -lroute_easy -lclientInput_manager -lpercent -lcookie_strMap -lsession_fileMap# insert here all dynamics libraries in DLIB_DIR_H you want to use

#########################
# general configurations
#########################
#CFLAGS          = -Wall -Wextra -g -Ofast -DNDEBUG -Wno-variadic-macros -fpermissive -fPIC -Wl,--export-dynamic # Werror transforms warning in error
CFLAGS          = -Wall -Wextra -g -Ofast -DNDEBUG -Wno-variadic-macros -fPIC -Wl,--export-dynamic -std=c++2a # standard support for c++17 - necessay to use std::variant 
DLIB_STD        = -lm -lpthread -lfcgi -lpqxx -lpq
DLIB            = $(DLIB_STD) $(DLIB_NAME)
COMPILER_FLAGS  = $(CFLAGS) $(DLIB_DIR_LPATH) $(DLIB_DIR_H_IPATH)
LINK_FLAGS      = $(COMPILER_FLAGS) $(DLIB_DIR_RPATH) # use -Wl,-rpath= when the library is not in global environment
LINK_DLIB       = $(LINK_FLAGS) -shared -Wl,-soname,$(LIB)
C_SRC_LIB       = $(PAGE) $(GLOBAL) $(DB_OBJ)
C_SRC_MAIN      = main.cpp
C_SRC           = $(C_SRC_LIB)
C_OBJ_ORI       = $(C_SRC:.cpp=.o)
C_OBJ_MAIN_ORI  = $(C_SRC_MAIN:.cpp=.o)
C_SRC_NAME_ONLY = $(notdir $(C_SRC))
C_OBJ_NAME_ONLY = $(C_SRC_NAME_ONLY:.cpp=.o)
C_OBJ_DIR       = objs/
C_OBJ           = $(addprefix $(C_OBJ_DIR), $(C_OBJ_NAME_ONLY))
C_LIB_H         = $(C_SRC_LIB:.cpp=.h)
#LIB             = lib$(C_SRC_LIB:.cpp=.so) # not necessary to up file to server - only in case to make dlib
EXE             = db++
PATH_SERVER     = /var/www/cgi-bin/

################################################
# CONFIGURATIONS OF HTML, JS, ETC.. FILES
################################################
HTML_SERVER_DIR = /var/www/html/pet
HTML_LOCAL_DIR = html
HTML_SUBDIR_TEMP_FILE = $(wildcard $(HTML_LOCAL_DIR)/*/*~) # cria uma lista de todos os arquivos temporários localizados em subdiretórios diretos em HTML_LOCAL_DIR, não pega subdiretórios dos subdiretórios 

#SUBDIRS = $(wildcard */.) # lista todos os diretórios do diretório atual
#all: $(SUBDIRS)
#$(SUBDIRS):
#        $(MAKE) -C $@
#.PHONY: all $(SUBDIRS)

################################################
# CLEAN TEMPORARY FILES
################################################
TEMP_FILE = $(wildcard */*~) # lista de todos os arquivos temporários em subdiretórios diretos do diretório local, não pega subdiretórios dos subdiretórios


################################################
# INCLUDE LIBRARIES OF THE LIBRARY
################################################
PAGE = 
DB_OBJ = 
################################################
# INCLUDE ARGS
################################################
# ARG1       = -input test/main.cpp#-q input.dat
ARG1       = -input test/file_input_text2.txt
ARG2       = -output test/main_out.cpp#-o output.dat
# ARG3       = -database_connection="dbname=pet user=borges password=JSG3bor_g873sqlptgs78b hostaddr=127.0.0.1 port=5432"#-i list_log_files.dat
# ARG3	= -d="dbname=security user=borges password=JSG3bor_g873sqlptgs78b hostaddr=127.0.0.1 port=5432"
ARG4       = --verbose#-d date.dat
ARG5       = #-e str_end_block.dat

#define DATABASE_CONNECTION "dbname=pet user=borges password=JSG3bor_g873sqlptgs78b hostaddr=127.0.0.1 port=5432"
#define DATABASE_CONNECTION_SECURITY "dbname=security user=borges password=JSG3bor_g873sqlptgs78b hostaddr=127.0.0.1 port=5432"

# up: html linker
# 	$(info $n$nINFO - sever: nginx$naddr of application in browser$nhttp://localhost:8080/cgi-bin/$(EXE))
# 	$(info INFO - server: apache2$naddr of application in browser$nhttp://localhost/cgi-bin/$(EXE))
# 	sudo cp $(EXE) $(PATH_SERVER)

run: linker
	$(info $nrun: $(EXE))
	./$(EXE) $(ARG1) $(ARG2) $(ARG3) $(ARG4) $(ARG5)

#lib: linker_lib export_lib export_lib_header
#$(info Dynamic Library created with success: $(LIB) $nDynamic Library Header exported with success: $(C_LIB_H)$nTo use: #include <headers/$(C_LIB_H)>)	
# can use to print: $(info your_text) $(warning your_text) or $(error your_text) # for new/break line use: $nYour_text - ex: my_text_line1 $nmy_text_line2

#linker_lib: cscrean clean_lib $(C_SRC:.cpp=.o) mv_c_obj
#	$(info $nlinkier objects to produce: $(LIB))
#	$(CC) $(LINK_DLIB) $(C_OBJ) -o $(LIB) $(DLIB)

linker: cscrean add_c_src_main $(C_SRC:.cpp=.o) $(C_SRC_MAIN:.cpp=.o)
	$(info $nlinker objects to produce: $(EXE))
	$(CC) $(LINK_FLAGS) $(C_OBJ_ORI) -o $(EXE) $(DLIB)
#$(CC) $(LINK_FLAGS) $(C_OBJ_ORI) $(C_OBJ_MAIN_ORI) -o $(EXE) $(DLIB)

$(C_SRC:.cpp=.o): %.o : %.cpp
	$(info $ncompile: $<)
	$(CC) $(COMPILER_FLAGS) -c $< -o $@ $(DLIB)

$(C_SRC_MAIN:.cpp=.o): %.o : %.cpp
	$(info $ncompile: $<)
	$(CC) $(COMPILER_FLAGS) -c $< -o $@ $(DLIB)
	
export_lib:
	$(info $nexport lib and lib_header:)
	cp $(LIB) $(DLIB_DIR)/

export_lib_header:
	cp $(C_LIB_H) $(DLIB_DIR_H)/headers/

mv_c_obj: 
	mv $(C_OBJ_ORI) $(C_OBJ_DIR)

# only in C_SRC variable is necessary to concatenate whit C_SRC_MAIN, all others just loading them again
add_c_src_main:
	$(info add files from C_SRC_MAIN to global compile variables)
	$(eval C_SRC += $(C_SRC_MAIN))
	$(info new makefile variable: C_SRC = $(C_SRC))
	$(eval C_OBJ_ORI = $(C_SRC:.cpp=.o))
	$(info new makefile variable: C_OBJ_ORI = $(C_OBJ_ORI))
	$(eval C_SRC_NAME_ONLY = $(notdir $(C_SRC)))
	$(info new makefile variable: C_SRC_NAME_ONLY = $(C_SRC_NAME_ONLY))
	$(eval C_OBJ_NAME_ONLY = $(C_SRC_NAME_ONLY:.cpp=.o))
	$(info new makefile variable: C_OBJ_NAME_ONLY = $(C_OBJ_NAME_ONLY))
	$(eval C_OBJ = $(addprefix $(C_OBJ_DIR), $(C_OBJ_NAME_ONLY)))
	$(info new makefile variable: C_OBJ = $(C_OBJ))

clean_lib: clean
	rm -rf $(LIB) $(DLIB_DIR)/$(LIB) $(DLIB_DIR_H)/headers/$(C_LIB_H)

clean:
	rm -rf $(C_OBJ_DIR)*.o *~ *.out *.key out.txt $(TEMP_FILE) $(C_OBJ_ORI) $(C_OBJ_MAIN_ORI)

cscrean:
	clear

valg: compile
	valgrind ./$(EXE) $(ARG1) $(ARG2)

# copy html local to /var/www
html: cscrean
	rm -rf $(HTML_SUBDIR_TEMP_FILE)
	sudo rm -rf $(HTML_SERVER_DIR)/*
	sudo cp -R $(HTML_LOCAL_DIR)/* $(HTML_SERVER_DIR)/
	

define n # define a break line - new line to user's message


endef

























	
