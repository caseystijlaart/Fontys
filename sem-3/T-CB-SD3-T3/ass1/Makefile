SUBDIRS := $(wildcard */.)

all: $(SUBDIRS)
	$(foreach f, $^, $(MAKE) -C $(f) $@;)

test: $(SUBDIRS)
	$(foreach f, $^, $(MAKE) -C $(f) $@;)

clean: $(SUBDIRS)
	$(foreach f, $^, $(MAKE) -C $(f) $@;)

.PHONY: all test $(SUBDIRS)
