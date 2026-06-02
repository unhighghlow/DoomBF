all: ibf frnt doom.bpk

.PHONY: all

doom.bpk: bfk_doom.elf
	python3 RISC-BF/risc_bf.py -c bfk_doom.elf doom.bpk

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

