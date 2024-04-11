PRODUCT=MyQueue

CPPFLAGS=-Iproduct -Itest -I../myStack/product
CXXFLAGS=-Wall -Werror -Wextra -pedantic -ggdb -O0 $(CPPFLAGS)

SOURCES=$(wildcard product/*.cpp) \
        $(wildcard test/*.cpp) \
		$(wildcard ../myStack/product/*.cpp)

HEADERS=$(wildcard product/*.h) \
        $(wildcard test/*.h) \
		$(wildcard ../myStack/product/*.h)

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
