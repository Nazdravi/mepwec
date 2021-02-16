CC = gcc
DBGFLAGS = -ggdb
CFLAGS   = -std=gnu99 -fplan9-extensions $(DBGFLAGS) -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS  =
LIBS     =
INCLUDE  = 
TARGET   = playersvc
OBJDIR   = ./obj
ifeq "$(strip $(OBJDIR))" ""
  OBJDIR = .
endif
SOURCES  = playersvc.c
LIBS = -lpthread -lreadline -lcrypt
OBJECTS  = $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))
HEADERS  = $(SOURCES:.c=.h) specific-commands.h

INCLUDE += -I./

$(TARGET): $(OBJECTS)
	$(CC) $(LIBS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.c $(HEADERS)
	@[ -d $$(dirname $@) ] || mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

all: clean $(TARGET)

clean:
	-rm -f $(OBJECTS) $(TARGET)
	-@rmdir --ignore-fail-on-non-empty $$(readlink -f $(OBJDIR))

.PHONY: copy
copy:
	for f in $(SOURCES) $(HEADERS); do scp $${f} pi@media-pi.slueterufer:Entwicklung/PlayerSvc/; done

.PHONY: run
run:
	./$(TARGET)

.PHONY: doc
doc:
	doxygen doxygen.conf

