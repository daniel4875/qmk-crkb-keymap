#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT_split_3x6_3(KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC, KC_ESC, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT, KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_LGUI, MO(1), KC_LCTL, KC_SPC, MO(2), KC_LALT),
	[1] = LAYOUT_split_3x6_3(KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_MINS, KC_EQL, KC_LBRC, KC_RBRC, KC_NUHS, KC_QUOT, KC_LSFT, KC_NUBS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_LGUI, KC_TRNS, KC_LCTL, KC_SPC, MO(3), KC_LALT),
	[2] = LAYOUT_split_3x6_3(KC_NO, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_DEL, KC_CAPS, KC_NO, KC_NO, KC_NO, KC_F11, KC_F12, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_NO, KC_INS, KC_LSFT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_HOME, KC_END, KC_PGUP, KC_PGDN, KC_NO, KC_RSFT, KC_LGUI, MO(3), KC_LCTL, KC_SPC, KC_TRNS, KC_LALT),
	[3] = LAYOUT_split_3x6_3(KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_PSCR, KC_MPRV, TG(4), KC_NO, KC_NO, KC_NO, KC_NO, KC_BRIU, KC_VOLU, KC_NO, KC_NO, KC_NO, KC_NO, KC_MNXT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_BRID, KC_VOLD, KC_MUTE, KC_NO, KC_NO, KC_NO, KC_MPLY, KC_NO, KC_TRNS, KC_NO, KC_NO, KC_TRNS, KC_NO),
	[4] = LAYOUT_split_3x6_3(KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_PSLS, KC_P7, KC_P8, KC_P9, KC_PMNS, KC_BSPC, TG(4), KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_PAST, KC_P4, KC_P5, KC_P6, KC_PPLS, KC_PENT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_P1, KC_P2, KC_P3, KC_PEQL, KC_NUM, KC_NO, KC_NO, KC_NO, KC_NO, KC_P0, KC_PDOT)
};

#ifdef OLED_ENABLE

// Animation parameters
#define MATRIX_ANIM_FRAME_DURATION 10 // frame duration in ms
#define MATRIX_SPAWN_CHAIN_PERCENT 20 // percentage chance of a new character chain spawning in an empty column in a given frame

// RNG parameters
#define RAND_ADD 53
#define RAND_MUL 181
#define RAND_MOD 167

// RNG variables
uint16_t random_value = 157;

// Animation variables
uint32_t matrix_anim_timer = 0;

// Matrix variables
uint8_t next_bottom_of_col[5] = {0};
uint8_t top_of_col[5] = {0};
uint8_t col_remaining_chain_length[5] = {0};
uint8_t min_chain_length = 5;
uint8_t max_chain_length = 16; // Note: OLED can fit 16 chars along height, so max chain length greater than 16 is out of range of screen (0 to 15)

static uint8_t generate_random_number(uint8_t max_num) {
    // Generate next value in sequence to use as pseudo-random number
    random_value = ((random_value * RAND_MUL) + RAND_ADD) % RAND_MOD;

    // Get first 8 bits of one of the system clocks, TCNT1
    uint8_t clockbyte = TCNT1 % 256;

    // Combine pseudo-random number with clock byte to make a theoretically even-more-random number,
    // then limit to range of 0 to max_num
    return (random_value ^ clockbyte) % max_num;
}

static char generate_random_char(void) {
    static const char matrix_chars[] = "!#$%&()0123456789<>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]abcdefghijklmnopqrstuvwxyz{}";
    return matrix_chars[generate_random_number(strlen(matrix_chars))];
}

// Check if any column of OLED doesn't contain any falling chars
// Note: OLED can fit 5 chars across width, so we have 5 columns
static bool col_without_chars_exists(void) {
    bool exists = false;
    for (uint8_t col = 0; col < 5; col++) {
        if (next_bottom_of_col[col] == 0) {
            exists = true;
        }
    }
    return exists;
}

// Check if the top char is a space for all columns of OLED
// Note: OLED can fit 5 chars across width, so we have 5 columns
static bool all_cols_have_space_at_top(void) {
    bool all_have_space_at_top = true;
    for (uint8_t col = 0; col < 5; col++) {
        if ((top_of_col[col] == 0) && (next_bottom_of_col[col] != 0)) {
            all_have_space_at_top = false;
        }
    }
    return all_have_space_at_top;
}

// Choose random column of OLED that doesn't contain any falling chars
// Note: OLED can fit 5 chars across width, so we have 5 columns
static uint8_t choose_random_col_without_chars(void) {
    uint8_t available_cols[5] = {0};
    uint8_t num_cols_available = 0;
    for (uint8_t col = 0; col < 5; col++) {
        if (next_bottom_of_col[col] == 0) {
            available_cols[num_cols_available] = col;
            num_cols_available++;
        }
    }
    uint8_t rand_num = generate_random_number(num_cols_available);
    return available_cols[rand_num];
}

// Render next frame of matrix digital rain animation
static void render_matrix_digital_rain_frame(void) {
    // Add new char chain to random column that hasn't already got chars in it (not every frame, only a chance per frame)
    bool should_add_new_char = (generate_random_number(100) + 1) <= MATRIX_SPAWN_CHAIN_PERCENT;
    if (all_cols_have_space_at_top()) should_add_new_char = true; // Ensure new char chain is created if all cols have space at top (to prevent big gaps in rain)
    if (should_add_new_char && col_without_chars_exists()) {
        // Choose random chain length
        uint8_t chain_length_range = max_chain_length - min_chain_length + 1;
        uint8_t chain_length = min_chain_length + generate_random_number(chain_length_range);
        // Choose random column
        uint8_t col = choose_random_col_without_chars();
        // Store chain length for column
        col_remaining_chain_length[col] = chain_length;
    }
    
    // Remove top char (replace top char with space) for all columns that have reached their chain length
    // (and actually have chars in it)
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t next_space_pos = top_of_col[col];
        uint8_t chain_length_remaining = col_remaining_chain_length[col];
        if ((chain_length_remaining == 0) && (next_bottom_of_col[col] > 0)) {
            oled_set_cursor(col, next_space_pos);
            oled_write_char(' ', false);
            top_of_col[col]++;
        }
    }
    
    // Add new char to all columns that haven't reached their chain length yet, or already contain chars and haven't reached the bottom of the screen
    // Note: OLED can fit 16 chars along height, so if next bottom of col is 16, it is out of range of screen (0 to 15)
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t next_char_pos = next_bottom_of_col[col];
        uint8_t chain_length_remaining = col_remaining_chain_length[col];
        if ((chain_length_remaining > 0) || ((next_char_pos > 0) && (next_char_pos < 16))) {
            char new_char = generate_random_char();
            oled_set_cursor(col, next_char_pos);
            oled_write_char(new_char, false);
            next_bottom_of_col[col]++;
            if (chain_length_remaining > 0)
                col_remaining_chain_length[col]--;
        }
    }

    // Reset columns that have had all chars removed (replaced with spaces)
    for (int col = 0; col < 5; col++) {
        if (top_of_col[col] == 16) {
            next_bottom_of_col[col] = 0;
            top_of_col[col] = 0;
        }
    }
}

static void render_matrix_digital_rain(void) {
    // Run animation
    if (timer_elapsed32(matrix_anim_timer) > MATRIX_ANIM_FRAME_DURATION) {
        // Set timer to updated time
        matrix_anim_timer = timer_read32();

        // Draw next frame
        render_matrix_digital_rain_frame();
    }

    /* Note:
     * timer_read32() -> gives ms it has been since keyboard was powered on
     * timer_elapsed32(timer) -> gives ms since given time "timer"
     */
}

static void render_layer_state(void) {
    oled_write_P(PSTR("LAYER"), false);
    oled_set_cursor(0, 2);

    switch (get_highest_layer(layer_state)) {
        case 0:
            oled_write_ln_P(PSTR("Base"), false);
            break;
        case 1:
            oled_write_ln_P(PSTR("NmSym"), false);
            break;
        case 2:
            oled_write_ln_P(PSTR("FnNav"), false);
            break;
        case 3:
            oled_write_ln_P(PSTR("Media"), false);
            break;
        case 4:
            oled_write_ln_P(PSTR("NumPd"), false);
            break;
        default:
            oled_write_ln_P(PSTR("Undef"), false);
            break;
    }
}

static void render_caps_state(void) {
    oled_set_cursor(0, 6);
    oled_write_P(PSTR("CAPS"), false);
    oled_set_cursor(0, 8);
    if (host_keyboard_led_state().caps_lock) {
        oled_write_ln_P(PSTR("On"), false);
    } else {
        oled_write_ln_P(PSTR("Off"), false);
    }
}

static void render_wpm(void) {
    oled_set_cursor(0, 12);
    oled_write_P(PSTR("WPM"), false);
    oled_set_cursor(0, 14);
    oled_write_ln(get_u8_str(get_current_wpm(), '0'), false);
}

static void render_status(void) {
    render_layer_state();
    render_caps_state();
    render_wpm();
}

// Set OLED rotation
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}

// Draw to OLED
bool oled_task_user(void) {
    // If OLED is off but last input time is less than timeout, turn OLED on
    if (!is_oled_on() && last_input_activity_elapsed() < OLED_TIMEOUT) {
        oled_on();
    }
    
    // If OLED is on but last input time is more than timeout, turn OLED off
    if (is_oled_on() && last_input_activity_elapsed() >= OLED_TIMEOUT) {
        oled_off();
    }
    
    // If OLED is on then draw to it
    if (is_oled_on()) {
        // Draw to left OLED
        if (is_keyboard_master()) {
            render_status();
        // Draw to right OLED
        } else {
            render_matrix_digital_rain();
        }
    }
    
    return false;
}

#endif

