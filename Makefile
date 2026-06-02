all: ibf frnt doom.bpk

.PHONY: all

doom.bpk: bfk_doom.elf
	python3 ./RISC-BF/asm.py -c bfk_doom.elf doom.bpk

lnx_doom: doom
	$(MAKE) -C $^ $@
	cp $^/$@ .

fake_bfk_doom: doom
	$(MAKE) -C $^ $@
	cp $^/$@ .

bfk_doom.elf: doom
	$(MAKE) -C $^ $@
	cp $^/$@ .

ibf: industrial-bf
	$(MAKE) -C $^ $@
	cp $^/$@ .

frnt: frontend
	$(MAKE) -C $^ $@
	cp $^/$@ .

