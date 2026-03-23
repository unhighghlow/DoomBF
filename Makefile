doom.b: bf-tcc doom
	BFCC=$(abspath bf-tcc) $(MAKE) -C doom $@
	cp doom/$@ .

all: lnx_doom ibf hackablebf bf-tcc doom.b

.PHONY: all

lnx_doom: doom
	$(MAKE) -C $^ $@
	cp $^/$@ .

fake_bfk_doom: doom
	$(MAKE) -C $^ $@
	cp $^/$@ .

ibf: bf/industrial-bf
	$(MAKE) -C $^ $@
	cp $^/$@ .

hackablebf: bf/hackablebf
	$(MAKE) -C $^ $@
	cp $^/$@ .

bf-tcc: tcc
	$(MAKE) -C $^ $@
	cp $^/$@ .

frnt: frontend
	$(MAKE) -C $^ $@
	cp $^/$@ .
