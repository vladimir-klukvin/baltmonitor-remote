CC = gcc
CFLAGS = -Wall
LDFLAGS = -lpthread

OBJDIR = obj
SRCDIR = src

SRC := $(shell find $(SRCDIR) -name "*.c")
OBJ := $(SRC:%.c=$(OBJDIR)/%.obj)

APP = baltmonitor-remote

all: CFLAGS += -Ofast
all: $(APP)

release: CFLAGS += -DNDEBUG -Ofast
release: $(APP)

debug: CFLAGS += -DDEBUG -g
debug: $(APP)

$(APP): $(OBJ)
	@$(CC) $^ $(LDFLAGS) -o $(APP)

$(OBJDIR)/%.obj: %.c
	@mkdir -p '$(@D)'
	@$(CC) -c $(CFLAGS) $< -o $@

clean:
	find . -name *.o -delete
	rm -f $(APP)