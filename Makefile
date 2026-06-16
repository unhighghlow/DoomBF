all: ibf frnt doom.bpk

run: all
	rm .pipe || true
	mkfifo .pipe
	./ibf -ac doom.bpk < .pipe | ./frnt > .pipe

clean:
	rm -f ./ibf ./frnt ./doom.bpk
	$(MAKE) -C doom $@
	$(MAKE) -C industrial-bf $@
	$(MAKE) -C frontend $@

rebuild: clean all

.PHONY: all run clean rebuild

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

