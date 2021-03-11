CC = gcc
CFLAGS = -Wall
LDFLAGS = -lpthread

OBJDIR = obj
SRCDIR = src

SRC := $(shell find $(SRCDIR) -name "*.c")
OBJ := $(SRC:%.c=$(OBJDIR)/%.o)

APP = baltmonitor-remote

all: CFLAGS += -DNDEBUG -O3
all: $(APP)

release: CFLAGS += -DNDEBUG -O3
release: $(APP)

debug: CFLAGS += -DDEBUG -g
debug: $(APP)

$(APP): $(OBJ)
	@$(CC) $^ $(LDFLAGS) -o $(APP)

$(OBJDIR)/%.o: %.c
	@mkdir -p '$(@D)'
	@$(CC) -c $(CFLAGS) $< -o $@

clean:
	find . -name *.o -delete
	rm -f $(APP)