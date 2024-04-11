PRODUCT=ass6_1

CXXFLAGS=-Wall -Werror -Wextra -pedantic -ggdb -O0 $(CPPFLAGS)

SOURCES=$(wildcard *.cpp) 

HEADERS=$(wildcard *.h) 

CXX=g++

.PHONY: all 

all: $(PRODUCT)

$(PRODUCT): $(SOURCES) $(HEADERS)
	@$(CXX) $(CXXFLAGS) $(SOURCES) -o $@

clean:
	@rm -rf $(PRODUCT) out.txt

test: $(PRODUCT)
	./test
