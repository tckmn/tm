TARGET = bin/tm
PREFIX ?= /usr/local
.PHONY: all clean debug release install

all: $(TARGET)

bin/%.o: src/%.c $(wildcard src/*.h)
	@mkdir -p bin
	gcc $(FLAGS) -Wall -Wextra -Wpedantic `pkg-config --cflags --libs libnotify` -c $< -o $@

$(TARGET): $(patsubst src/%.c, bin/%.o, $(wildcard src/*.c))
	@mkdir -p bin
	gcc $(FLAGS) -Wall -Wextra -Wpedantic `pkg-config --cflags --libs libnotify` $^ -o $@

debug: FLAGS = -g

debug: $(TARGET)

release: FLAGS = -Os

release: $(TARGET)
	strip -s -R .comment -R .gnu.version $(TARGET)

install: $(TARGET)
	install -D $(TARGET) $(DESTDIR)$(PREFIX)/$(TARGET)
	install -Dm644 tm.1 $(DESTDIR)$(PREFIX)/share/man/man1/tm.1

clean:
	rm -rf bin
