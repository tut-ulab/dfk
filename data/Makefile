.PHONY: all

PATH := ../make-index

D-LEARN := D-learn.dat
D-VALID := D-valid.dat
D-TEST := D-test.dat
D-LEARN-1000 := D-learn-1000.dat
D-VALID-1000 := D-valid-1000.dat
D-TEST-1000 := D-test-1000.dat

#DATA := $(D-LEARN) $(D-VALID) $(D-TEST) $(D-LEARN-1000) $(D-VALID-1000) $(D-TEST-1000)
DATA := $(D-LEARN-1000) $(D-VALID-1000) $(D-TEST-1000)
CLASSES := $(DATA:.dat=.class)


#all:
#	$(echo $PATH)
all: $(CLASSES)
	/bin/echo $(CLASSES)


D-learn-1000.dat: D-learn.dat
	/usr/bin/head -n 1000 $< > $@

D-valid-1000.dat: D-valid.dat
	/usr/bin/head -n 1000 $< > $@

D-test-1000.dat: D-test.dat
	/usr/bin/head -n 1000 $< > $@


# data
D-learn.class: $(D-LEARN)
	$(PATH)/class-detect $< | $(PATH)/class-sort > $@

D-valid.class: $(D-VALID)
	$(PATH)/class-detect $< | $(PATH)/class-sort > $@

D-test.class: $(D-TEST)
	$(PATH)/class-detect $< | $(PATH)/class-sort > $@

D-learn-1000.class: $(D-LEARN-1000)
	$(PATH)/class-detect $< | $(PATH)/class-sort > $@

D-valid-1000.class: $(D-VALID-1000)
	$(PATH)/class-detect $< | $(PATH)/class-sort > $@

D-test-1000.class: $(D-TEST-1000)
	$(PATH)/class-detect $< | $(PATH)/class-sort > $@
