doom.b: bf-tcc doom
	BFCC=$(abspath bf-tcc) $(MAKE) -C doom $@
	cp doom/$@ .

all: lnx_doom fake_bfk_doom bfk_doom.elf ibf hackablebf doom.b

.PHONY: all

lnx_doom: doom
	$(MAKE) -C $^ $@
	cp $^/$@ .

fake_bfk_doom: doom
	$(MAKE) -C $^ $@
	cp $^/$@ .

bfk_doom.elf: doom
	$(MAKE) -C $^ $@
	cp $^/$@ .

ibf: bf/industrial-bf
	$(MAKE) -C $^ $@
	cp $^/$@ .

hackablebf: bf/hackablebf
	$(MAKE) -C $^ $@
	cp $^/$@ .

frnt: frontend
	$(MAKE) -C $^ $@
	cp $^/$@ .
