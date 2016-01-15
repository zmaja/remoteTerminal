IDIR=.
CC=g++
CFLAGS=-I$(IDIR) -pthread # -DNDEBUG
	
SRCDIR=.

OBJDIR=/home/gnezdo/Documents/screenCast/exec

LIBS=-ljpeg

_DEPS = Common.h ProcessingNode.h SocketInputNode.h DecompressAndDisplayNode.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.cpp Common.cpp ProcessingNode.cpp SocketInputNode.cpp DecompressAndDisplayNode.cpp
#OBJ = $(patsubst %,$(SRCDIR)/%,$(_OBJ))

OBJS := $(addprefix $(OBJDIR)/,$(_OBJ:.cpp=.o))

$(OBJDIR)/%.o : %.cpp $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

screenrcv: $(OBJS)
	$(CC) -o $(OBJDIR)/$@ $^ $(CFLAGS) $(LIBS)

$(OBJS): | $(OBJDIR)
$(OBJDIR):
	mkdir $(OBJDIR)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/screenrcv $(OBJDIR)/*.o *~ core $(IDIR)/*~
