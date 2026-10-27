#!/usr/bin/env bash
# Run host checks using cJSON from the selected official ESP-IDF checkout.
set -euo pipefail
cd "$(dirname "$0")/.."
: "${IDF_PATH:?Activate ESP-IDF v5.5.5 first}"
mkdir -p firmware/tests/out
python3 scripts/generate-device-art.py --check
python3 scripts/generate-avatar-art.py --check
python3 scripts/generate-species-art.py --check
python3 firmware/ui/companion/build_assets.py --verify
python3 firmware/ui/theme/build_assets.py --verify
python3 firmware/ui/scenes/build_assets.py --verify
node --input-type=module -e "import {initialState} from './src/model.js';import {projectDevice,serializeDevice} from './src/device-export.js';process.stdout.write(serializeDevice(projectDevice({...initialState(),avatar:{skin:2,hair:1,cloak:2,style:1}}).packet))" > firmware/tests/out/demo.json
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core -I"$IDF_PATH/components/json/cJSON" firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/core/combat.c firmware/core/dice_pool.c firmware/core/packet.c firmware/tests/test_core.c "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o firmware/tests/out/core
firmware/tests/out/core firmware/tests/out/demo.json
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core -I"$IDF_PATH/components/json/cJSON" firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/core/combat.c firmware/core/dice_pool.c firmware/core/packet.c firmware/tests/checks_probe.c "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o firmware/tests/out/checks-probe
node scripts/check-training.js
node scripts/hardware-session.js --check
cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core -I"$IDF_PATH/components/json/cJSON" firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/core/packet.c firmware/tests/wallet_probe.c "$IDF_PATH/components/json/cJSON/cJSON.c" -lm -o firmware/tests/out/wallet-probe
node scripts/check-wallet.js
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
cc -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter -Ifirmware/core -Ifirmware/tests/fake firmware/core/history.c firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/tests/test_history_store.c -o firmware/tests/out/history-storage
cc -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter -Ifirmware/core -Ifirmware/tests/fake firmware/core/history.c firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/tests/test_restore_storage.c -o firmware/tests/out/restore-storage
firmware/tests/out/restore-storage
firmware/tests/out/history-storage
firmware/tests/out/storage
cmake -S firmware/tests -B firmware/tests/out/ui-build -G Ninja
cmake --build firmware/tests/out/ui-build --parallel 4
(cd firmware/tests/out/ui-build && ./dice && ./pool && ./avatar && ./appearance && ./wallet && ./vitals && ./quantities && ./power && ./catalog && ./history && ./diagnostics diagnostics-codec.json && ./input-check && ./backup ../backup-fixture.json && ./checks && ./combat && ./magic && ./preparation && ./rest && ./status && ./death-roll && ./effects && ./durations && ./damage && ./death-damage && ./concentration && ./links && ./temporary-defences && ./mixed-damage && ./conditional-defences && ./multiple-defences && ./reductions && ./reduction-profiles && ./profile-costs && ./slot-costs && ./turns && ./external-turns && ./turn-names && ./time && ./charges && ./multi-costs && ./alternative-costs && ./recharge && ./roll-conditions && ./screen && ./buttons && ./alloc && ./ui)
firmware/tests/out/ui-build/avatar --hashes > firmware/tests/out/avatar-hashes.txt
firmware/tests/out/ui-build/avatar --species-hashes > firmware/tests/out/species-hashes.txt
node scripts/check-avatar.js
python3 scripts/check-device.py

cc -std=c11 -Wall -Wextra -Werror -Ifirmware/core firmware/core/backup.c firmware/core/history.c firmware/core/pocketdd.c firmware/core/magic.c firmware/core/checks.c firmware/tests/backup_probe.c -o firmware/tests/out/backup-probe
node scripts/check-backup.js
python3 scripts/check-device-backup.py
python3 scripts/check-device-check.py
