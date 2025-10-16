set(DOOM_SRC
    "${CMAKE_SOURCE_DIR}/doom/am_map.c"
    "${CMAKE_SOURCE_DIR}/doom/DOOM.c"
    "${CMAKE_SOURCE_DIR}/doom/doomdef.c"
    "${CMAKE_SOURCE_DIR}/doom/doomstat.c"
    "${CMAKE_SOURCE_DIR}/doom/dstrings.c"
    "${CMAKE_SOURCE_DIR}/doom/d_items.c"
    "${CMAKE_SOURCE_DIR}/doom/d_main.c"
    "${CMAKE_SOURCE_DIR}/doom/d_net.c"
    "${CMAKE_SOURCE_DIR}/doom/f_finale.c"
    "${CMAKE_SOURCE_DIR}/doom/f_wipe.c"
    "${CMAKE_SOURCE_DIR}/doom/g_game.c"
    "${CMAKE_SOURCE_DIR}/doom/hu_lib.c"
    "${CMAKE_SOURCE_DIR}/doom/hu_stuff.c"
    "${CMAKE_SOURCE_DIR}/doom/info.c"
    "${CMAKE_SOURCE_DIR}/doom/i_net.c"
    "${CMAKE_SOURCE_DIR}/doom/i_sound.c"
    "${CMAKE_SOURCE_DIR}/doom/i_system.c"
    "${CMAKE_SOURCE_DIR}/doom/i_video.c"
    "${CMAKE_SOURCE_DIR}/doom/m_argv.c"
    "${CMAKE_SOURCE_DIR}/doom/m_bbox.c"
    "${CMAKE_SOURCE_DIR}/doom/m_cheat.c"
    "${CMAKE_SOURCE_DIR}/doom/m_fixed.c"
    "${CMAKE_SOURCE_DIR}/doom/m_menu.c"
    "${CMAKE_SOURCE_DIR}/doom/m_misc.c"
    "${CMAKE_SOURCE_DIR}/doom/m_random.c"
    "${CMAKE_SOURCE_DIR}/doom/m_swap.c"
    "${CMAKE_SOURCE_DIR}/doom/p_ceilng.c"
    "${CMAKE_SOURCE_DIR}/doom/p_doors.c"
    "${CMAKE_SOURCE_DIR}/doom/p_enemy.c"
    "${CMAKE_SOURCE_DIR}/doom/p_floor.c"
    "${CMAKE_SOURCE_DIR}/doom/p_inter.c"
    "${CMAKE_SOURCE_DIR}/doom/p_lights.c"
    "${CMAKE_SOURCE_DIR}/doom/p_map.c"
    "${CMAKE_SOURCE_DIR}/doom/p_maputl.c"
    "${CMAKE_SOURCE_DIR}/doom/p_mobj.c"
    "${CMAKE_SOURCE_DIR}/doom/p_plats.c"
    "${CMAKE_SOURCE_DIR}/doom/p_pspr.c"
    "${CMAKE_SOURCE_DIR}/doom/p_saveg.c"
    "${CMAKE_SOURCE_DIR}/doom/p_setup.c"
    "${CMAKE_SOURCE_DIR}/doom/p_sight.c"
    "${CMAKE_SOURCE_DIR}/doom/p_spec.c"
    "${CMAKE_SOURCE_DIR}/doom/p_switch.c"
    "${CMAKE_SOURCE_DIR}/doom/p_telept.c"
    "${CMAKE_SOURCE_DIR}/doom/p_tick.c"
    "${CMAKE_SOURCE_DIR}/doom/p_user.c"
    "${CMAKE_SOURCE_DIR}/doom/r_bsp.c"
    "${CMAKE_SOURCE_DIR}/doom/r_data.c"
    "${CMAKE_SOURCE_DIR}/doom/r_draw.c"
    "${CMAKE_SOURCE_DIR}/doom/r_main.c"
    "${CMAKE_SOURCE_DIR}/doom/r_plane.c"
    "${CMAKE_SOURCE_DIR}/doom/r_segs.c"
    "${CMAKE_SOURCE_DIR}/doom/r_sky.c"
    "${CMAKE_SOURCE_DIR}/doom/r_things.c"
    "${CMAKE_SOURCE_DIR}/doom/sounds.c"
    "${CMAKE_SOURCE_DIR}/doom/st_lib.c"
    "${CMAKE_SOURCE_DIR}/doom/st_stuff.c"
    "${CMAKE_SOURCE_DIR}/doom/s_sound.c"
    "${CMAKE_SOURCE_DIR}/doom/tables.c"
    "${CMAKE_SOURCE_DIR}/doom/v_video.c"
    "${CMAKE_SOURCE_DIR}/doom/wi_stuff.c"
    "${CMAKE_SOURCE_DIR}/doom/w_wad.c"
    "${CMAKE_SOURCE_DIR}/doom/z_zone.c"
)
set(CRT_SRC
    "${CMAKE_SOURCE_DIR}/crt/heap.c"
    "${CMAKE_SOURCE_DIR}/crt/printf.c"
    "${CMAKE_SOURCE_DIR}/crt/doom_crt.c"
)

if (WIN32)
    set(INC_OPTS "-I" "${CMAKE_SOURCE_DIR}/bin/include")
    set(LINK_OPTS -luser32 -lgdi32)
    set(FRONTEND_SRC "${CMAKE_CURRENT_SOURCE_DIR}/win_doom.c")
    set(OUT "${CMAKE_BINARY_DIR}/mini_doom.exe")

    add_custom_command(
        OUTPUT ${OUT}
        #COMMAND chcp 65001
        COMMAND ${TCC_EXE} -o ${OUT} ${DOOM_SRC} ${CRT_SRC} ${FRONTEND_SRC} -I${CMAKE_SOURCE_DIR} ${INC_OPTS} -L${CMAKE_SOURCE_DIR}/bin ${LINK_OPTS}
        DEPENDS ${SRC}
        COMMENT "\n~~~~ Building mini_doom with tcc"
    )

    add_custom_target(
        mini_doom ALL
        DEPENDS ${OUT}
    )

    add_custom_target(clean_mini_doom ALL
        COMMAND ${CMAKE_COMMAND} -E rm -f "${OUT}"
        COMMENT "\n~~~~ Remove old mini_doom"
    )

    add_custom_command(TARGET mini_doom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OUT}" "${CMAKE_SOURCE_DIR}/bin/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/data/doom.wad" "${CMAKE_SOURCE_DIR}/bin/"
        COMMENT "\n~~~~ Copying WAD file to bin directory"
    )

    add_dependencies(mini_doom tcc clean_mini_doom)

else()
    set(INC_OPTS -I ${CMAKE_SOURCE_DIR})
    set(LINK_OPTS -lX11)
    set(FRONTEND_SRC "${CMAKE_CURRENT_SOURCE_DIR}/lnx_doom.c")
    set(OUT "mini_doom")
    
    add_executable(${OUT} ${DOOM_SRC} ${CRT_SRC} ${FRONTEND_SRC})
    #target_compile_options(${OUT} PRIVATE -Wno-pointer-to-int-cast)
    #target_compile_options(${OUT} PRIVATE -m32)
    target_link_libraries(${OUT} PRIVATE ${LINK_OPTS})
    target_include_directories(${OUT} PRIVATE ${INC_OPTS})
    add_custom_command(TARGET ${OUT} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${OUT}> "${CMAKE_SOURCE_DIR}/bin/")
endif()

