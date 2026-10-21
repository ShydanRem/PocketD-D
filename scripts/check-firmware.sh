#!/usr/bin/env bash
# Run host checks using cJSON from the selected official ESP-IDF checkout.
set -euo pipefail
cd "$(dirname "$0")/.."
: "${IDF_PATH:?Activate ESP-IDF v5.5.5 first}"
mkdir -p firmware/tests/out
python3 scripts/generate-device-art.py --check
node --input-type=module -e "import {initialState} from './src/model.js';import {projectDevice,serializeDevice} from './src/device-export.js';process.stdout.write(serializeDevice(projectDevice({...initialState(),avatar:{skin:2,hair:1,cloak:2,style:1}}).packet))" > firmware/tests/out/demo.json
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core -I"$IDF_PATH/components/json/cJSON" firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/core/combat.c firmware/core/dice_pool.c firmware/core/packet.c firmware/tests/test_core.c "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o firmware/tests/out/core
firmware/tests/out/core firmware/tests/out/demo.json
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core -I"$IDF_PATH/components/json/cJSON" firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/core/combat.c firmware/core/dice_pool.c firmware/core/packet.c firmware/tests/checks_probe.c "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o firmware/tests/out/checks-probe
node scripts/check-training.js
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core -I"$IDF_PATH/components/json/cJSON" firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/core/combat.c firmware/core/dice_pool.c firmware/core/packet.c firmware/tests/combat_probe.c "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o firmware/tests/out/combat-probe
node scripts/check-combat.js
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core -I"$IDF_PATH/components/json/cJSON" firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/core/packet.c firmware/tests/magic_probe.c "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o firmware/tests/out/magic-probe
node scripts/check-magic.js
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core -I"$IDF_PATH/components/json/cJSON" firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/core/dice_pool.c firmware/core/rest.c firmware/core/packet.c firmware/tests/rest_probe.c "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o firmware/tests/out/rest-probe
node scripts/check-rest.js
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core -I"$IDF_PATH/components/json/cJSON" firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/core/dice_pool.c firmware/core/rest.c firmware/core/packet.c firmware/tests/charges_probe.c "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o firmware/tests/out/charges-probe
node scripts/check-charges.js
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core -I"$IDF_PATH/components/json/cJSON" firmware/core/pocketdd.c firmware/core/status.c firmware/core/magic.c firmware/core/checks.c firmware/core/combat.c firmware/core/dice_pool.c firmware/core/rest.c firmware/core/packet.c firmware/tests/status_probe.c "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o firmware/tests/out/status-probe
node scripts/check-status.js
cc -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter -Ifirmware/core -Ifirmware/tests/fake firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/core/combat.c firmware/core/dice_pool.c firmware/tests/test_storage.c -o firmware/tests/out/storage
firmware/tests/out/storage
cmake -S firmware/tests -B firmware/tests/out/ui-build -G Ninja
cmake --build firmware/tests/out/ui-build --parallel 4
(cd firmware/tests/out/ui-build && ./dice && ./pool && ./avatar && ./checks && ./combat && ./magic && ./rest && ./status && ./effects && ./durations && ./damage && ./concentration && ./links && ./temporary-defences && ./mixed-damage && ./conditional-defences && ./multiple-defences && ./reductions && ./reduction-profiles && ./profile-costs && ./slot-costs && ./turns && ./time && ./charges && ./multi-costs && ./recharge && ./screen && ./buttons && ./alloc && ./ui)
python3 scripts/check-device.py
