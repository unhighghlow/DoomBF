CFLAGS = -std=c11 -g

DOOM_FILES_REL = \
        am_map   d_items  \
        d_main   d_net    \
        doomdef  doomstat \
        dstrings f_finale \
        f_wipe   g_game   \
        hu_lib   hu_stuff \
        i_net    info     \
        i_sound  i_system \
        i_video  m_argv   \
        m_bbox   m_cheat  \
        m_fixed  m_menu   \
        m_misc   m_random \
        m_swap   p_ceilng \
        p_doors  p_enemy  \
        p_floor  p_inter  \
        p_lights p_map    \
        p_maputl p_mobj   \
        p_plats  p_pspr   \
        p_saveg  p_setup  \
        p_sight  p_spec   \
        p_switch p_telept \
        p_tick   p_user   \
        r_bsp    r_data   \
        r_draw   r_main   \
        r_plane  r_segs   \
        r_sky    r_things \
        sounds   s_sound  \
        st_lib   st_stuff \
        tables   v_video  \
        wi_stuff w_wad    \
        z_zone   DOOM
DOOM_FILES = $(patsubst %, backend/%, $(DOOM_FILES_REL))
DOOM_OBJECTS = $(patsubst %, %.o, $(DOOM_FILES))
DOOM_SOURCES = $(patsubst %, %.c, $(DOOM_FILES))

CRT_FILES = crt/doom_crt crt/heap crt/printf
CRT_OBJECTS = $(patsubst %, %.o, $(CRT_FILES))
CRT_SOURCES = $(patsubst %, %.c, $(CRT_FILES))

lnx_doom : LDFLAGS = -lX11
lnx_doom: lnx_doom.o $(DOOM_OBJECTS) $(CRT_OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

bfk_doom.o: doom_wad.h

bfk_doom.elf: bfk_doom.c doom_wad.h $(DOOM_SOURCES) $(CRT_SOURCES)
	riscv32-none-elf-gcc \
	    -O3 \
	    -march=rv32i \
	    -mabi=ilp32 \
	    -ffreestanding \
	    -nostdlib \
	    -T bfk_link.ld \
	    -std=c11 \
	    -D_BF \
	    bfk_doom.c $(DOOM_SOURCES) $(CRT_SOURCES) \
	    -lgcc \
	    -o bfk_doom.elf

fake_bfk_doom: bfk_doom.o $(DOOM_OBJECTS) $(CRT_OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

doom.b: bfk_doom.c doom_wad.h $(DOOM_SOURCES) $(CRT_SOURCES)
	$(BFCC) $(CFLAGS) $^ -o $@

doom_wad.h: data/doom.wad
	xxd -i $^ doom_wad.h
