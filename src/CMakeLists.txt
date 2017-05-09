set(CALICO_SOURCES
    am_main.c
    comnjag.c
    d_main.c
                    doomdata.h
                    doomdef.h
    f_main.c
    g_game.c
    info.c
    info.h
    in_main.c
    jagcry.c        jagcry.h
    jagonly.c
                    jagpad.h
    j_eeprom.c
                    keywords.h
    m_argv.c        m_argv.h
    m_main.c
                    music.h
    o_main.c
    p_base.c
    p_ceilng.c
    p_change.c
    p_doors.c
    p_enemy.c
    p_floor.c
    p_inter.c
    p_lights.c
                    p_local.h
    p_map.c
    p_maputl.c
    p_mobj.c
    p_move.c
    p_plats.c
    p_pspr.c
    p_setup.c
    p_shoot.c
    p_sight.c
    p_slide.c
    p_spec.c        p_spec.h
    p_switch.c
    p_telept.c
    p_tick.c
    p_user.c
    r_data.c        r_local.h
    r_main.c
    r_phase1.c
    r_phase2.c
    r_phase3.c
    r_phase4.c
    r_phase5.c
    r_phase6.c
    r_phase7.c
    r_phase8.c
    r_phase9.c
                    sound.h
    sounds.c        sounds.h
    soundst.h
    sprinfo.c
    s_sound.c
    s_soundfmt.cpp  s_soundfmt.h
    st_main.c       st_main.h
    tables.c
    vsprintf.c
    w_iwad.c        w_iwad.h
    w_wad.c
    z_zone.c)

set(ELIB_SOURCES
    elib/atexit.cpp     elib/atexit.h
                        elib/bdlist.h
                        elib/binary.h
                        elib/compare.h
    elib/configfile.cpp elib/configfile.h
                        elib/dllist.h
                        elib/elib.h
                        elib/m_ctype.h
    elib/misc.cpp       elib/misc.h
    elib/parser.cpp     elib/parser.h
    elib/qstring.cpp    elib/qstring.h
                        elib/swap.h
    elib/zone.cpp       elib/zone.h)

set(GL_SOURCES
    gl/gl_render.cpp    gl/gl_render.h
    gl/resource.cpp     gl/resource.h)

set(HAL_SOURCES
    hal/hal_init.c      hal/hal_init.h
    hal/hal_input.c     hal/hal_input.h
    hal/hal_ml.c        hal/hal_ml.h
    hal/hal_platform.c  hal/hal_platform.h
    hal/hal_sfx.c       hal/hal_sfx.h
    hal/hal_timer.c     hal/hal_timer.h
                        hal/hal_types.h
    hal/hal_video.c     hal/hal_video.h)

set(POSIX_SOURCES
    posix/posix_main.c
    posix/posix_platform.cpp    posix/posix_platform.h)

set(RB_SOURCES
                        rb/rb_common.h
    rb/rb_draw.cpp      rb/rb_draw.h
    rb/rb_main.cpp      rb/rb_main.h
    rb/rb_texture.cpp   rb/rb_texture.h
                        rb/rb_types.h
    rb/valloc.cpp       rb/valloc.h)

set(SDL_SOURCES
    sdl/sdl_hal.c       sdl/sdl_hal.h
    sdl/sdl_init.c      sdl/sdl_init.h
    sdl/sdl_input.cpp   sdl/sdl_input.h
    sdl/sdl_sound.cpp   sdl/sdl_sound.h
    sdl/sdl_timer.cpp   sdl/sdl_timer.h
    sdl/sdl_video.cpp   sdl/sdl_video.h)

add_executable(calico-doom
    ${CALICO_SOURCES} ${ELIB_SOURCES} ${GL_SOURCES} ${HAL_SOURCES}
    ${POSIX_SOURCES} ${RB_SOURCES} ${SDL_SOURCES})
target_compile_definitions(calico-doom PRIVATE "-DUSE_SDL2")
target_link_libraries(calico-doom SDL2::SDL2 SDL2::mixer ${OPENGL_gl_LIBRARY})
