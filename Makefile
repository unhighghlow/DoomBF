all: ibf frnt doom.bpk

run: all
	rm .pipe || true
	mkfifo .pipe
	./ibf -c doom.bpk < .pipe | ./frnt > .pipe

.PHONY: all run

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

ibf: industrial-bf
	$(MAKE) -C $^ $@
	cp $^/$@ .

frnt: frontend
	$(MAKE) -C $^ $@
	cp $^/$@ .

