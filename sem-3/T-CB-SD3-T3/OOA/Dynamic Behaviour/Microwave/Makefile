PRODUCT=StateTest

CPPFLAGS=-Iproduct -Itest
CXXFLAGS=-Wall -Werror -Wextra -pedantic -ggdb -O0 $(CPPFLAGS)

SOURCES=$(wildcard product/*.cpp) \
        $(wildcard test/*.cpp)

HEADERS=$(wildcard product/*.h) \
        $(wildcard test/*.h)

LDFLAGS=-lgtest -lgmock -lgmock_main -lpthread

CXX=g++

.PHONY: all clean test

all: $(PRODUCT)

$(PRODUCT): $(SOURCES) $(HEADERS)
	@$(CXX) $(CXXFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

clean:
	@rm -rf $(PRODUCT)

test: $(PRODUCT)
	@./$(PRODUCT)
