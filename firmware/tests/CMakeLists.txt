cmake_minimum_required(VERSION 3.16)
project(pocketdd_host_tests C CXX)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_LIST_DIR}/lv_conf.h" CACHE PATH "" FORCE)
add_subdirectory(../.deps/lvgl lvgl)
add_executable(ui ../core/recharge.c ../core/damage.c ../core/screen.c ../core/status.c ../core/rest.c test_ui.c ../core/buttons.c ../core/pocketdd.c ../core/magic.c ../core/checks.c ../core/combat.c ../core/dice_animation.c ../core/dice_pool.c ../core/avatar.c)
target_include_directories(ui PRIVATE ../core)
target_link_libraries(ui lvgl m)

# Isolated visual experiment; does not replace or link into the shipping UI.
option(PD_BUILD_COMPANION_PROTOTYPE "Build the optional native companion UI prototype" OFF)
if(PD_BUILD_COMPANION_PROTOTYPE)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(COMPANION_DIR "${CMAKE_CURRENT_LIST_DIR}/../prototypes/companion")
    set(COMPANION_ASSETS "${CMAKE_CURRENT_BINARY_DIR}/companion-assets")
    add_custom_command(
        OUTPUT "${COMPANION_ASSETS}/companion_assets.c" "${COMPANION_ASSETS}/companion_assets.h"
        COMMAND ${Python3_EXECUTABLE} "${COMPANION_DIR}/build_assets.py" "${COMPANION_ASSETS}"
        DEPENDS "${COMPANION_DIR}/build_assets.py" "${COMPANION_DIR}/source/camp.png"
                "${COMPANION_DIR}/source/hero.png" "${COMPANION_DIR}/source/icons.png"
        VERBATIM)
    add_executable(companion "${COMPANION_DIR}/companion.c" "${COMPANION_DIR}/test_companion.c"
        "${COMPANION_ASSETS}/companion_assets.c" ../core/pocketdd.c ../core/magic.c ../core/checks.c)
    target_include_directories(companion PRIVATE ../core "${COMPANION_DIR}" "${COMPANION_ASSETS}")
    target_compile_options(companion PRIVATE -Wall -Wextra -Werror)
    target_link_libraries(companion lvgl m)
endif()

add_executable(dice test_dice.c ../core/dice_animation.c ../core/pocketdd.c ../core/magic.c ../core/checks.c ../core/dice_pool.c)
target_include_directories(dice PRIVATE ../core)
add_executable(pool test_pool.c ../core/dice_pool.c ../core/dice_animation.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(pool PRIVATE ../core)
add_executable(avatar test_avatar.c ../core/avatar.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(avatar PRIVATE ../core)
add_executable(checks test_checks.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(checks PRIVATE ../core)
add_executable(combat test_combat.c ../core/pocketdd.c ../core/magic.c ../core/checks.c ../core/combat.c ../core/dice_pool.c)
target_include_directories(combat PRIVATE ../core)
add_executable(magic test_magic.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(magic PRIVATE ../core)
add_executable(buttons test_buttons.c ../core/buttons.c)
target_include_directories(buttons PRIVATE ../core)
add_executable(alloc test_alloc.c ../core/magic.c ../core/checks.c)
target_include_directories(alloc PRIVATE ../core)

add_executable(rest test_rest.c ../core/rest.c ../core/pocketdd.c ../core/magic.c ../core/checks.c ../core/dice_pool.c)
target_include_directories(rest PRIVATE ../core)

add_executable(status test_status.c ../core/status.c ../core/rest.c ../core/combat.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(status PRIVATE ../core)

add_executable(screen test_screen.c ../core/screen.c)
target_include_directories(screen PRIVATE ../core)

add_executable(effects test_effects.c ../core/pocketdd.c ../core/magic.c ../core/checks.c ../core/status.c ../core/combat.c ../core/rest.c ../core/dice_pool.c)
target_include_directories(effects PRIVATE ../core)

add_executable(durations test_durations.c ../core/pocketdd.c ../core/magic.c ../core/checks.c ../core/rest.c ../core/dice_pool.c)
target_include_directories(durations PRIVATE ../core)

add_executable(damage test_damage.c ../core/damage.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(damage PRIVATE ../core)

add_executable(concentration test_concentration.c ../core/damage.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(concentration PRIVATE ../core)

add_executable(links test_links.c ../core/damage.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(links PRIVATE ../core)

add_executable(temporary-defences test_temporary_defences.c ../core/damage.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(temporary-defences PRIVATE ../core)

add_executable(mixed-damage test_mixed_damage.c ../core/damage.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(mixed-damage PRIVATE ../core)

add_executable(conditional-defences test_conditional_defences.c ../core/damage.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(conditional-defences PRIVATE ../core)

add_executable(multiple-defences test_multiple_defences.c ../core/damage.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(multiple-defences PRIVATE ../core)

add_executable(reductions test_reductions.c ../core/damage.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(reductions PRIVATE ../core)

add_executable(reduction-profiles test_reduction_profiles.c ../core/damage.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(reduction-profiles PRIVATE ../core)

add_executable(profile-costs test_profile_costs.c ../core/damage.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(profile-costs PRIVATE ../core)

add_executable(slot-costs test_slot_costs.c ../core/damage.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(slot-costs PRIVATE ../core)

add_executable(turns test_turns.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(turns PRIVATE ../core)

add_executable(time test_time.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(time PRIVATE ../core)

add_executable(charges test_charges.c ../core/damage.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(charges PRIVATE ../core)

add_executable(multi-costs test_multi_costs.c ../core/damage.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(multi-costs PRIVATE ../core)


add_executable(recharge test_recharge.c ../core/recharge.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(recharge PRIVATE ../core)

add_executable(roll-conditions test_roll_conditions.c ../core/status.c ../core/rest.c ../core/combat.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(roll-conditions PRIVATE ../core)
