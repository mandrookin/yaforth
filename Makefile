#
#
#
ifeq ($(OS),Windows_NT)
  PLUGINS := 
else
  PLUGINS := 
endif

SRC := yaforth.cpp middleware.cpp codegen.cpp

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
#	$(CXX) $^ -l$(XLIB) -o yaforth
else
all: $(OBJECTS)
	$(CXX) $^ $(DEBUG) -o yaforth
#	$(CXX) $^ $(DEBUG) -l$(XLIB) -o yaforth
endif

#check:
#	./slagheap.exe -ansi -devops:1 lib/asm 2>&1 || (echo "Assembling test not pass $$?"; exit 1)

clean:
	rm -f $(OBJECTS) yaforth
#	echo "Delete binary executable objects"
#	find . -name "*.xod"  -type f -delete
#	echo "Delete MIF and listings"
#	find ./ -type f \( -iname \*.mif -o -iname \*.mif_dbg \) -delete
	rm -f yaforth
	rm -rf $(ROOTOBJ)


$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(DEBUG) -I$(INCLUDES) -c $< -o $@

$(OBJDIR):
	mkdir $@




