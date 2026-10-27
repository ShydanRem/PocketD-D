cmake_minimum_required(VERSION 3.16)
project(pocketdd_host_tests C CXX)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_LIST_DIR}/lv_conf.h" CACHE PATH "" FORCE)
add_subdirectory(../.deps/lvgl lvgl)
add_executable(ui ../core/catalog.c ../core/power.c ../core/input_check.c ../core/diagnostics.c ../core/backup.c ../core/history.c ../core/recharge.c ../core/damage.c ../core/screen.c ../core/status.c ../core/rest.c test_ui.c ../core/buttons.c ../core/pocketdd.c ../core/magic.c ../core/checks.c ../core/combat.c ../core/dice_animation.c ../core/dice_pool.c ../core/avatar.c)
target_sources(ui PRIVATE ../ui/scenes/scenes.c ../ui/scenes/scene_assets.c ../ui/theme/theme.c ../ui/theme/theme_assets.c ../ui/companion/companion.c ../ui/companion/companion_assets.c)
target_include_directories(ui PRIVATE ../core ../ui/companion ../ui/theme)
target_link_libraries(ui lvgl m)

# Standalone view harness, using the same production renderer and committed assets.
option(PD_BUILD_COMPANION_PROTOTYPE "Build the companion view harness" OFF)
if(PD_BUILD_COMPANION_PROTOTYPE)
    add_executable(companion ../ui/companion/companion.c ../ui/companion/companion_assets.c
        ../prototypes/companion/test_companion.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
    target_include_directories(companion PRIVATE ../core ../ui/companion)
    target_compile_options(companion PRIVATE -Wall -Wextra -Werror)
    target_link_libraries(companion lvgl m)
endif()

add_executable(dice test_dice.c ../core/dice_animation.c ../core/pocketdd.c ../core/magic.c ../core/checks.c ../core/dice_pool.c)
target_include_directories(dice PRIVATE ../core)
add_executable(pool test_pool.c ../core/dice_pool.c ../core/dice_animation.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(pool PRIVATE ../core)
add_executable(appearance test_appearance.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(appearance PRIVATE ../core)
add_executable(avatar test_avatar.c ../core/avatar.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
add_executable(wallet test_wallet.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
add_executable(quantities test_quantities.c ../core/pocketdd.c ../core/magic.c ../core/checks.c ../core/combat.c ../core/dice_pool.c ../core/history.c ../core/backup.c)
target_include_directories(quantities PRIVATE ../core)
target_compile_options(quantities PRIVATE -Wall -Wextra -Werror)
add_executable(vitals test_vitals.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(vitals PRIVATE ../core)
target_compile_options(vitals PRIVATE -Wall -Wextra -Werror)
target_include_directories(wallet PRIVATE ../core)
target_include_directories(avatar PRIVATE ../core)
add_executable(checks test_checks.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(checks PRIVATE ../core)
add_executable(combat test_combat.c ../core/pocketdd.c ../core/magic.c ../core/checks.c ../core/combat.c ../core/dice_pool.c)
target_include_directories(combat PRIVATE ../core)
add_executable(magic test_magic.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
add_executable(preparation test_preparation.c ../core/magic.c ../core/pocketdd.c ../core/checks.c ../core/rest.c ../core/dice_pool.c ../core/backup.c ../core/history.c)
target_include_directories(preparation PRIVATE ../core)
target_compile_options(preparation PRIVATE -Wall -Wextra -Werror)
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

add_executable(history test_history.c ../core/history.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(history PRIVATE ../core)

add_executable(backup test_backup.c ../core/backup.c ../core/history.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(backup PRIVATE ../core)

add_executable(diagnostics test_diagnostics.c ../core/diagnostics.c)
target_include_directories(diagnostics PRIVATE ../core)
target_compile_options(diagnostics PRIVATE -Wall -Wextra -Werror)

add_executable(input-check test_input_check.c ../core/input_check.c ../core/buttons.c)
target_include_directories(input-check PRIVATE ../core)
target_compile_options(input-check PRIVATE -Wall -Wextra -Werror)

add_executable(death-damage test_death_damage.c ../core/damage.c ../core/status.c ../core/rest.c ../core/dice_pool.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(death-damage PRIVATE ../core)
target_compile_options(death-damage PRIVATE -Wall -Wextra -Werror)

add_executable(death-roll test_death_roll.c ../core/status.c ../core/history.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(death-roll PRIVATE ../core)
target_compile_options(death-roll PRIVATE -Wall -Wextra -Werror)

add_executable(external-turns test_external_turns.c ../core/backup.c ../core/history.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(external-turns PRIVATE ../core)
target_compile_options(external-turns PRIVATE -Wall -Wextra -Werror)

add_executable(turn-names test_turn_names.c ../core/backup.c ../core/history.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(turn-names PRIVATE ../core)
target_compile_options(turn-names PRIVATE -Wall -Wextra -Werror)

add_executable(alternative-costs test_alternative_costs.c ../core/damage.c ../core/status.c ../core/backup.c ../core/history.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(alternative-costs PRIVATE ../core)
target_compile_options(alternative-costs PRIVATE -Wall -Wextra -Werror)

add_executable(power test_power.c ../core/power.c)
target_include_directories(power PRIVATE ../core)
target_compile_options(power PRIVATE -Wall -Wextra -Werror)

add_executable(catalog test_catalog.c ../core/catalog.c ../core/pocketdd.c ../core/magic.c ../core/checks.c)
target_include_directories(catalog PRIVATE ../core)
target_compile_options(catalog PRIVATE -Wall -Wextra -Werror)

target_link_libraries(appearance m)
