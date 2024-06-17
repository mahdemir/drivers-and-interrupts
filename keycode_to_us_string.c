/* 
 * US keymap
 * I got this from: https://github.com/jarun/spy/blob/master/spy.c 
 */
static const char *us_keymap[][2] = {
	{"\0", "\0"}, {"ESC", "ESC"}, {"1", "!"}, {"2", "@"},       // 0-3
	{"3", "#"}, {"4", "$"}, {"5", "%"}, {"6", "^"},                 // 4-7
	{"7", "&"}, {"8", "*"}, {"9", "("}, {"0", ")"},                 // 8-11
	{"-", "_"}, {"=", "+"}, {"BACKSPACE", "BACKSPACE"},         // 12-14
	{"TAB", "TAB"}, {"q", "Q"}, {"w", "W"}, {"e", "E"}, {"r", "R"},
	{"t", "T"}, {"y", "Y"}, {"u", "U"}, {"i", "I"},                 // 20-23
	{"o", "O"}, {"p", "P"}, {"[", "{"}, {"]", "}"},                 // 24-27
	{"RETURN", "RETURN"}, {"LCTRL", "LCTRL"}, {"a", "A"}, {"s", "S"},   // 28-31
	{"d", "D"}, {"f", "F"}, {"g", "G"}, {"h", "H"},                 // 32-35
	{"j", "J"}, {"k", "K"}, {"l", "L"}, {";", ":"},                 // 36-39
	{"'", "\""}, {"`", "~"}, {"LSHIFT", "LSHIFT"}, {"\\", "|"}, // 40-43
	{"z", "Z"}, {"x", "X"}, {"c", "C"}, {"v", "V"},                 // 44-47
	{"b", "B"}, {"n", "N"}, {"m", "M"}, {",", "<"},                 // 48-51
	{".", ">"}, {"/", "?"}, {"RSHIFT", "RSHIFT"}, {"PRTSCR", "KPD*"},
	{"LALT", "LALT"}, {" ", " "}, {"CAPS", "CAPS"}, {"F1", "F1"},
	{"F2", "F2"}, {"F3", "F3"}, {"F4", "F4"}, {"F5", "F5"},         // 60-63
	{"F6", "F6"}, {"F7", "F7"}, {"F8", "F8"}, {"F9", "F9"},         // 64-67
	{"F10", "F10"}, {"NUM", "NUM"}, {"SCROLL", "SCROLL"},   // 68-70
	{"KPD7", "HOME"}, {"KPD8", "UP"}, {"KPD9", "PGUP"}, // 71-73
	{"-", "-"}, {"KPD4", "LEFT"}, {"KPD5", "KPD5"},         // 74-76
	{"KPD6", "RIGHT"}, {"+", "+"}, {"KPD1", "END"},         // 77-79
	{"KPD2", "DOWN"}, {"KPD3", "PGDN"}, {"KPD0", "INS"}, // 80-82
	{"KPD.", "DEL"}, {"SYSRQ", "SYSRQ"}, {"\0", "\0"},      // 83-85
	{"\0", "\0"}, {"F11", "F11"}, {"F12", "F12"}, {"\0", "\0"},     // 86-89
	{"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},
	{"\0", "\0"}, {"KPENTER", "KPENTER"}, {"RCTRL", "RCTRL"}, {"/", "/"},
	{"PRTSCR", "PRTSCR"}, {"RALT", "RALT"}, {"\0", "\0"},   // 99-101
	{"HOME", "HOME"}, {"UP", "UP"}, {"PGUP", "PGUP"},   // 102-104
	{"LEFT", "LEFT"}, {"RIGHT", "RIGHT"}, {"END", "END"},
	{"DOWN", "DOWN"}, {"PGDN", "PGDN"}, {"INS", "INS"},   // 108-110
	{"DEL", "DEL"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},   // 111-114
	{"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},         // 115-118
	{"PAUSE", "PAUSE"},                                         // 119
};

// Returns the size of the string copied into the buffer, with its maximum being buff_size.
size_t keycode_to_us_string(int keycode, int shift, char *buffer, size_t buff_size, int *ascii_val)
{
	*ascii_val = 0;
    memset(buffer, 0x0, buff_size);

	if(keycode > KEY_RESERVED && keycode <= KEY_PAUSE) 
	{
	    // If shift is pressed we want a capital letter
		const char *us_key = (shift == 1) ? us_keymap[keycode][1] : us_keymap[keycode][0];
		snprintf(buffer, buff_size, "%s", us_key);
		// strlen(buffer) couldn't be greather than buff_size
		if (us_key[0] != '\0')
		{
			if (!strcmp(us_key, "ESC"))
				*ascii_val = 27;
			else if (!strcmp(us_key, "BACKSPACE"))
				*ascii_val = 8;
			else if (!strcmp(us_key, "TAB"))
				*ascii_val = 9;
			else if (!strcmp(us_key, "RETURN"))
				*ascii_val = 13;
			else if (!strcmp(us_key, "KPENTER"))
				*ascii_val = 10;
			else if (us_key[0] == '_' && us_key[1] == 'K' && us_key[2] == 'P' && us_key[3] == 'D')
				*ascii_val = us_key[4];
			else if (us_key[1] == '\0')
				*ascii_val = us_key[0];
			else
				*ascii_val = 999;
		}
		return strlen(buffer);
	}

	return 0;
}
