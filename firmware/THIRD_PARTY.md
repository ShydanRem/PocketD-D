# Third-party sources

`main/board.c` adapts the display command sequence and GPIO/touch initialization
of Waveshare's Apache-2.0 BSP in
https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/tree/e4344e70c2fa78a13e8a06566507f1ba8af6672a/firmware/brookesia/components/waveshare__esp32_s3_touch_amoled_1_75
Copyright Waveshare / Espressif. License retained in LICENSE-WAVESHARE.
Changes are identified in the source header. No factory application is copied.

Build dependencies are pinned in dependencies.json, fetched to ignored `.deps/`.
LVGL: MIT; Espressif esp-bsp touch / esp-iot-solution CO5300 and CMake utilities:
Apache-2.0; Waveshare CST9217: Apache-2.0. Each dependency's license and copyright
notices remain in its source checkout. ESP-IDF v5.5.5: Apache-2.0 plus component
licenses (including cJSON MIT). Distributed binary bundles must include these
licenses; the source URLs and revisions are retained for reproducibility.

LVGL's built-in Montserrat fonts also embed Font Awesome 5 glyphs. Montserrat:
Copyright 2011 The Montserrat Project Authors (original design Julieta Ulanovsky),
SIL Open Font License 1.1. Font Awesome 5 Free: Copyright Fonticons, Inc., font
portion SIL OFL 1.1 (additional icon/code terms in its notice). Upstream notices
are retained in `licenses/` and accompany source/binary bundles.

The polyhedral dice atlases and pixel numeral glyphs are original PocketD-D assets, generated
by `scripts/generate-dice.py` and rendered by `core/dice_animation.c`; they do
not depend on third-party images, fonts or online asset services.

The device scene, base adventurer and navigation icons in `assets/device_art.h`
are original PocketD-D pixel art authored by `scripts/generate-device-art.py`.
No third-party artwork or imported character appearance is used.
