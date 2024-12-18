# My Custom Keymap for the Corne Keyboard

This is my keymap for the Corne keyboard. It includes code for OLED displays, where the left display shows keyboard status (layer, caps lock and WPM) and the right display has a matrix digital rain effect.

## Installing the Keymap

Put all files in this repo under the directory `<keymap>` (with a keymap name of your choosing) inside the `keymaps` directory for the `crkbd` keyboard.

## Compiling the Firmware

To compile the firmware (substitute `<keymap>` with the name you gave the keymap):

```
qmk compile -kb crkbd -km <keymap>
```

## Flashing the Firmware

To flash the firmware to the keyboard (run this command once for each half of the keyboard) (substitute `<keymap>` with the name you gave the keymap):

```
qmk flash -kb crkbd -km <keymap>
```

## Modifying the Keymap

1. Generate a JSON version of the keymap (substitute `<keymap>` with the name you gave the keymap):
```
qmk c2json -kb crkbd -km <keymap> -o layout.json
```
2. Upload the `layout.json` file to https://config.qmk.fm/ and edit the layout using the online editor.
3. Download the keymap as a JSON file and replace `layout.json` with it.
4. Generate a C code version of the keymap:
```
qmk json2c -o layout.c layout.json
```
5. Overwrite the `keymaps` array in `keymap.c` with the `keymaps` array in `layout.c`.
6. Delete `layout.c` and `layout.json`.

## Frame Animation Example

For an example of rendering an animation consisting of a series of frames to an OLED screen, see the code below. This is in place of the matrix digital rain animation code. Use this if you want to have an animation made up from a series of frames instead of randomly generated matrix digital rain.

Note: Each frame is an array of hex values generated using the tool available at https://javl.github.io/image2cpp/ to generate hex from a 32x128 image.

``` C

// Animation parameters
#define ANIM_FRAME_DURATION 400 // frame duration in ms

// Animation variables
uint32_t anim_timer = 0;
uint8_t current_frame = 0;

static void render_animation(void) {

    static const char PROGMEM frame1[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0xb0, 0x78, 0xf8, 0x7c, 0x1c, 0x1c, 0x1c, 0x0c, 0x0c,
        ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
    };

    static const char PROGMEM frame2[] = {
        0x00, 0x80, 0x80, 0x00, 0x80, 0xf0, 0xf8, 0xac, 0x1e, 0xfe, 0x9f, 0x07, 0x07, 0x07, 0x03, 0x03,
        ...
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    const char* all_frames[] = {
        frame1,
        frame2
    };

    uint16_t frame_sizes[] = {
        sizeof(frame1),
        sizeof(frame2)
    };

    uint16_t num_frames = sizeof(all_frames) / sizeof(all_frames[0]);

    // Run animation
    if (timer_elapsed32(anim_timer) > ANIM_FRAME_DURATION) {
        // Set timer to updated time
        anim_timer = timer_read32();

        // Increment frame
        current_frame = (current_frame + 1) % num_frames;

        // Draw frame
        oled_write_raw_P(all_frames[current_frame], frame_sizes[current_frame]);
    }

    /* Note:
     * timer_read32() -> gives ms it has been since keyboard was powered on
     * timer_elapsed32(timer) -> gives ms since given time "timer"
     */
}

```
