# Third-party sources

The optional `prototypes/companion/source/` PNGs were generated with OpenAI
ImageGen for the new companion UI direction. Their prompts and provenance are
retained in `prototypes/companion/source/provenance.json`; original bytes are
kept intact. They are not the procedurally authored assets described below and
are not linked into the shipping firmware. ImageMagick 7 is used at build time
to export the prototype raster data; it is not a runtime dependency. No provider
credentials or personal character files are included.

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

The personalized avatar masks in `assets/avatar_atlas.h` and
`src/device-avatar-masks.json` are original integer-grid artwork from the same
generator. `src/avatar-palette.json` preserves the existing PocketD-D palettes.
The PC export preview and firmware share these masks and colors; no third-party
avatar images or D&D Beyond portraits are embedded.

The v0.22 physical-key mapping was checked against HARDWARE_REFERENCE.md and
firmware/brookesia/components/ButtonTest/ButtonTest.cpp at the same pinned
Waveshare commit. BOOT is active-low GPIO0; conditioned PWR is active-high
TCA9554 P4. PocketD-D implements its own bounded polling/debounce state machine
and preserves the other expander direction/output/polarity bits.

The v0.23 rest/hit-die implementation, views and test fixtures are PocketD-D
code. It reuses the original pixel dice renderer and adds no external artwork,
rules catalogue, runtime service or firmware dependency.

The v0.24 status/death-save logic, condition-note views and fixtures are original
PocketD-D code. Common condition names are labels only; no external rules text,
character JSON, artwork or new runtime dependency is bundled.

The v0.25 screen controller and input quarantine are original PocketD-D code.
Brightness uses the public esp_lcd_panel_co5300_set_brightness API in the pinned
Espressif component; its SPI implementation handles QSPI and DCS 0x51. No new
PMIC code, dependencies or external artwork is introduced.


The v0.26 local numeric modifier model, editor and fixtures are original
PocketD-D code. No rules catalogue, character data, external assets or new
runtime dependency is introduced.


The v0.27 explicit round-duration model, expiry preview and synthetic fixtures
are original PocketD-D code. No rules text, external assets, private character
data, new runtime services or dependencies are introduced.


The v0.28 typed-damage calculation, manual-defence editor and synthetic fixtures
are original PocketD-D code. Common damage-type names are labels; no rules
catalogue, private character data, external artwork or new dependency is bundled.


The v0.29 concentration-save validation, explicit DC option and atomic damage
candidate are original PocketD-D code. It reuses the existing pixel dice assets;
no new external art, rules catalogue, runtime service or dependency is bundled.


The v0.30 explicit effect/concentration binding, atomic end transitions and
synthetic PDD11 migration fixture are original PocketD-D work. No private
character data, rules catalogue, external artwork or dependency is introduced.


The v0.31 additional defence grants, shared-round preview and synthetic PDD12
fixture are original PocketD-D code. No external artwork, rules catalogue,
private character data, credentials, runtime service or dependency is bundled.


The v0.32 mixed-impact grouping, component editor and synthetic tests are
original PocketD-D code. No new rules catalogue, private character data,
external artwork, credential, runtime service or dependency is introduced.


The v0.33 bounded defence predicates, explicit source-fact editor and synthetic
PDD13 migration fixture are original PocketD-D work. No rules catalogue,
private character JSON, credential, external artwork or dependency is introduced.

La v0.34 aggiunge codice, test e UI originali per tre extra difensivi, senza
asset esterni. La fixture PDD14 è sintetica e proviene dall'encoder originale
v0.33 `7cc17f8`; non contiene dati Beyond o informazioni personali.

La v0.35 aggiunge codice, test e interfaccia originali per riduzioni esplicite
prima/dopo e conto del colpo. Nessun nuovo asset esterno o dato personale.

La v0.36 aggiunge codice/UI e test originali per profili di riduzione. Fixture
PDD15 sintetica dell'encoder v0.35 `c99bdf0`, senza dati Beyond o informazioni
personali. Nessun nuovo asset esterno.


La v0.37 aggiunge codice, UI e test originali per costi dei profili e reazione.
Fixture PDD16 sintetica prodotta dall'encoder originale v0.36 `885d9d2`, senza
JSON Beyond o dati personali. Nessun nuovo asset o dipendenza esterna.


La v0.38 aggiunge codice, UI e test originali per costi in slot ordinari/patto.
Fixture PDD17 sintetica generata dall'encoder v0.37 originale `1274857`, prima
delle modifiche; nessun dato personale, JSON Beyond, asset o dipendenza esterna.


La v0.39 aggiunge codice, UI e test originali per timer del proprio turno.
Fixture PDD18 sintetica dell'encoder v0.38 originale `807ccff`, prima delle
modifiche. Nessun dato personale, JSON Beyond, asset o dipendenza esterna.


La v0.40 aggiunge codice, UI e test originali per minuti/ore e tempo di gioco.
Fixture PDD19 sintetica dell'encoder v0.39 originale `70a194b`, prima delle
modifiche. Nessun dato personale, JSON Beyond, asset o dipendenza esterna.


La v0.41 aggiunge modello, UI, parser e test originali per cariche degli oggetti.
Fixture PDD20 sintetica prodotta dall'encoder originale v0.40 `6604bad`, prima
delle modifiche. Nessun dato personale, JSON Beyond, asset o dipendenza esterna.


La v0.42 aggiunge codice, UI e test originali per tre spese congiunte.
Fixture PDD21 sintetica dell'encoder originale v0.41 `048ef28`, prodotta prima
delle modifiche. Nessun dato personale, JSON Beyond, asset o dipendenza esterna.


La v0.43 aggiunge core, UI e test originali per ricariche parziali delle cariche
con i dadi pixel art esistenti. Nessun asset, fixture di formato, dato personale,
JSON Beyond o dipendenza esterna aggiunti.


La v0.44 aggiunge core, UI e test originali per quattro effetti di condizione
sui tiri. Fixture PDD22 sintetica generata dall'encoder originale v0.43
`b340a8c`, prima delle modifiche. Nessun dato personale, JSON Beyond, asset
o dipendenza esterna aggiunti.


La v0.45 aggiunge disegni originali su griglia (diorama, cornice, libro,
bisaccia, cristallo, falò), ritocca le maschere avatar e introduce lettere/cifre
pixel originali, generati da `scripts/generate-device-art.py`. Nessun font,
illustrazione o dipendenza esterna aggiunti. Le licenze LVGL/ESP-IDF preesistenti
restano nell'archivio. Nessun dato personale o JSON Beyond.


La v0.46 ridisegna scena e maschere avatar originali e aggiunge un alone
originale I4 con palette alpha, generato dallo stesso script. Nessun asset,
font o servizio esterno aggiunto. Il confronto usa le catture sintetiche della
v0.45 approvata e quelle attuali; Pillow è lo strumento host già usato per le
anteprime, non una dipendenza del firmware. Nessun dato personale o JSON Beyond.


La v0.47 aggiunge nove icone e due angoli pixel originali, generati dallo
script esistente, oltre a UI e test originali per il feedback transitorio dei
saldi. Nessun asset/font/servizio esterno o dipendenza del firmware aggiunti.
Le catture e la GIF del feedback usano dati sintetici. Nessun dato personale
o JSON Beyond incluso.
