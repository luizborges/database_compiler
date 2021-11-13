CC              = g++
DLIB_DIR_H      = /var/www/shared
DLIB_DIR        = $(DLIB_DIR_H)/dlibs
DLIB_DIR_LPATH  = $(foreach dir,$(DLIB_DIR),   -L$(dir)) # add prefix to all dir
DLIB_DIR_H_IPATH= $(foreach dir,$(DLIB_DIR_H), -I$(dir)) # add prefix to all dir
DLIB_DIR_RPATH  = $(foreach dir,$(DLIB_DIR),   -Wl,-rpath=$(dir)) # add prefix to all dir
DLIB_NAME       = -lutilpp -ldatabasepp #-lerror -lfileUtil -lmemoryManager -lstackTracer -larrayList_noSync -lmap_ArrayList_noSync -labstractFactoryCommon -lcweb -lcwebpp  #-ldatabasepp#-lclientOutput_strMap -lroute_easy -lclientInput_manager -lpercent -lcookie_strMap -lsession_fileMap# insert here all dynamics libraries in DLIB_DIR_H you want to use

################################################
# COMPILER AND LINKER FLAGS AND OPTIONS
################################################
#CFLAGS          = -Wall -g -O3 -DNDEBUG -Wno-variadic-macros -fPIC -Wl,--export-dynamic # Werror transforms warning in error
#CFLAGS          = -Wall -g -Ofast -DNDEBUG -Wno-variadic-macros -fPIC -Wl,--export-dynamic # change O3 to Ofast -> Specify -ofast to perform O3 optimizations plus disregard strict standards compliance.
### USE THIS FLAG FOR PRODUCTION -Ofast
#CFLAGS          = -Wall -g -Ofast -DNDEBUG -Wno-variadic-macros -fPIC -Wl,--export-dynamic -std=c++17 # standard support for c++17 - necessay to use std::variant 
### USE  THIS FLAG FOR TEST -Og -> debugging -O0 normal
CFLAGS          = -Wall -g -O0 -DNDEBUG -Wno-variadic-macros -fPIC -Wl,--export-dynamic -std=c++2a # TEST -Og
DLIB_STD        = -lm -lpthread -lpqxx -lpq #-lfcgi -lgc 
DLIB            = $(DLIB_STD) $(DLIB_NAME)
COMPILER_FLAGS  = $(CFLAGS) #$(DLIB_DIR_LPATH) $(DLIB_DIR_H_IPATH) # use $(DLIB_DIR_LPATH) $(DLIB_DIR_H_IPATH) when library is not in global environment
LINK_FLAGS      = $(COMPILER_FLAGS) #$(DLIB_DIR_RPATH) # use -Wl,-rpath= when the library is not in global environment
LINK_DLIB       = $(LINK_FLAGS) -shared -Wl,-soname,$(LIB)


C_SRC_LIB       = 
C_SRC_MAIN      = main.cpp
C_SRC           = $(C_SRC_LIB)
C_OBJ_ORI       = $(C_SRC:.cpp=.o)
C_SRC_NAME_ONLY = $(notdir $(C_SRC))
C_OBJ_NAME_ONLY = $(C_SRC_NAME_ONLY:.cpp=.o)
C_OBJ_DIR       = objs/
C_OBJ           = $(addprefix $(C_OBJ_DIR), $(C_OBJ_NAME_ONLY))
C_LIB_H         = $(C_SRC_LIB:.cpp=.h)
#LIB             = lib$(C_SRC_LIB:.cpp=.so) # not necessary to up file to server - only in case to make dlib
EXE             = testdba
PATH_SERVER     = /var/www/cgi-bin/


ARG1       = #-q input.dat
ARG2       = #-o output.dat
ARG3       = #-i list_log_files.dat
ARG4       = #-d date.dat
ARG5       = #-e str_end_block.dat


#up: linker
#	$(info $n$nINFO$naddr of application in browser$nhttp://localhost/cgi-bin/$(EXE))
#	sudo cp $(EXE) $(PATH_SERVER)

run: linker
	$(info $nrun: $(EXE))
	./$(EXE) $(ARG1) $(ARG2) $(ARG3) $(ARG4) $(ARG5)

#lib: linker_lib export_lib export_lib_header
#$(info Dynamic Library created with success: $(LIB) $nDynamic Library Header exported with success: $(C_LIB_H)$nTo use: #include <headers/$(C_LIB_H)>)	
# can use to print: $(info your_text) $(warning your_text) or $(error your_text) # for new/break line use: $nYour_text - ex: my_text_line1 $nmy_text_line2

#linker_lib: cscrean clean_lib $(C_SRC:.cpp=.o) mv_c_obj
#	$(info $nlinkier objects to produce: $(LIB))
#	$(CC) $(LINK_DLIB) $(C_OBJ) -o $(LIB) $(DLIB)

linker: cscrean clean add_c_src_main $(C_SRC:.cpp=.o) $(C_SRC_MAIN:.cpp=.o) mv_c_obj
	$(info $nlinker objects to produce: $(EXE))
	$(CC) $(LINK_FLAGS) $(C_OBJ) -o $(EXE) $(DLIB)

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
	rm -rf $(C_OBJ_DIR)*.o *~ *.out *.key out.txt

cscrean:
	clear

valg: compile
	valgrind ./$(EXE) $(ARG1) $(ARG2)
	
define n # define a break line - new line to user's message


endef

























	
