#
#
#
ifeq ($(OS),Windows_NT)
  PLUGINS := 
else
  PLUGINS := 
#  READLINE := -DUSE_READLINE -lreadline
#  $(shell /sbin/ldconfig -p | grep readline)
#  STATUS := $$? 
#  $(shell echo "Status is $(STATUS)")
endif          	

SRC := yaforth.cpp middleware.cpp codegen.cpp _main.cpp

ROOTOBJ := obj
OBJDIR := $(ROOTOBJ)
OBJECTS := $(SRC:%.cpp=$(OBJDIR)/%.o)
INCLUDES := include/
#SRC := $(wildcard $(SRC_DIR)/*.cc)

#DEBUG := -g

CXX	:= g++
#CXXFLAGS := -g
CXXFLAGS := -O3
#INCLUDES :=

ifeq ($(OS),Windows_NT)
all: $(OBJECTS)
	$(CXX) $^ -o yaforth
#	$(CXX) $^ -l$(LIBS) -o yaforth
else
all: $(OBJECTS)
#	$(CXX) $^ $(DEBUG) -o yaforth
	$(CXX) $^ $(DEBUG) $(READLINE) -o yaforth
endif

TESTS:=$(shell find tests -name *.frt)
check:
	@echo "Iterate overt test files in ./test dicrectory"
	@for forth_file in $(TESTS); do \
	    printf '##############  $(bold)$$forth_file$(sgr0) ###############' \
	    echo "##############  $$forth_file ###############"; \
	    ./yaforth -no-stdin $$forth_file; \
	done

bold := $(shell tput bold)
sgr0 := $(shell tput sgr0)


clean:
	rm -f $(OBJECTS) yaforth
#	echo "Delete binary executable objects"
#	find . -name "*.xod"  -type f -delete
#	echo "Delete MIF and listings"
#	find ./ -type f \( -iname \*.mif -o -iname \*.mif_dbg \) -delete
	find ./tests -type f \( -iname \*.asm -o -iname \*.mif_dbg \) -delete
	rm -f yaforth
	rm -rf $(ROOTOBJ)


$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(DEBUG) -I$(INCLUDES) -c $< -o $@

$(OBJDIR):
	mkdir $@




