all: lnx_doom ibf

.PHONY: all

lnx_doom:
	$(MAKE) -C doom
	cp doom/lnx_doom .

ibf:
	$(MAKE) -C bf/industrial-bf
	cp bf/industrial-bf/ibf .

hackablebf:
	$(MAKE) -C bf/hackablebf
	cp bf/hackablebf/hackablebf .
