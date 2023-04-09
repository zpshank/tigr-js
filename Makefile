
OBJDIR := .obj

CFLAGS := -I/usr/local/include/quickjs -Iextern
LDFLAGS := -L/usr/local/lib/quickjs
LDFLAGS += -lquickjs
LDFLAGS += -framework OpenGL -framework Cocoa

PROGS = examples/hello

.PHONY = clean

all: libtigrJS.a $(PROGS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $^

libtigrJS.a: $(OBJDIR)/src/tigrJS.o $(OBJDIR)/extern/tigr.o
	ar rcs $@ $^

$(OBJDIR):
	mkdir -p $(OBJDIR) $(OBJDIR)/src $(OBJDIR)/extern $(OBJDIR)/examples

examples/%: $(OBJDIR)/examples/%.o libtigrJS.a
	$(CC) -o $@ $^ $(LDFLAGS)

examples/%.c: examples/%.js $(OBJDIR)/src/tigrJS.o
	qjsc -e -M tigr,tigr -o $@ $<

clean:
	rm -rf $(OBJDIR) libtigrJS.a $(PROGS)
