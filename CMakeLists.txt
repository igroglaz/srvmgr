cmake_minimum_required(VERSION 3.10)

project(srvmgr)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

link_libraries(ws2_32)

add_library(srvmgr SHARED srvmgr.def
    a2server_utils.cpp
    a2types.cpp
    antiinvis.cpp
    asm_unit_info.cpp
    autobuff.cpp
    charcheck.cpp
    cheat_codes.cpp
    cheat_codes_new.cpp
    circle.cpp
    common_new.cpp
    config_new.cpp
    crash_filter.cpp
    crash_fixes.cpp
    crashes_new.cpp
    CRC_32.cpp
    ctl_handle.cpp
    distance_bug.cpp
    File.cpp
    forbidden_items.cpp
    inn.cpp
    itemex.cpp
    magic_resist_limits.cpp
    map_rotation.cpp
    multiplayer_shop.cpp
    new_character.cpp
    partial_drop.cpp
    pktmgr.cpp
    player_info.cpp
    protolayer_hat.cpp
    pvm2.cpp
    reborn_readiness.cpp
    scanrange.cpp
    scanrange_check.cpp
    screenshots.cpp
    scroll_burn.cpp
    shared.cpp
    shops.cpp
    solo.cpp
    spell_duration_fix.cpp
    srvmgr.cpp
    srvmgr_new.cpp
    startup.cpp
    summon.cpp
    this_call.cpp
    quests.cpp
    unit_info.cpp
    utils.cpp
    zxmgr.cpp
    lib/hat2.cpp
    lib/packet.cpp
    lib/ScanrangeCalc.cpp
    lib/serialize.cpp
    lib/socket.cpp
    lib/utils.cpp
)

# Static link.
set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
target_compile_options(srvmgr PUBLIC /MT)
target_link_options(srvmgr PUBLIC /NODEFAULTLIB:MSVCRT)

add_compile_definitions(srvmgr _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)

# Postbuild commands.
add_custom_command(TARGET srvmgr
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND copy build\\Release\\srvmgr.dll postbuild
)

add_custom_command(TARGET srvmgr
    POST_BUILD
    COMMAND cmd /c ${CMAKE_CURRENT_SOURCE_DIR}/postbuild/srvmgr.bat
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/postbuild
)
