const uint16_t Data_Audio00[] PROGMEM = {
	0x0045,0x0184,0x0005,0x0218,0x011c,0x0120,0x0124,0x012c,0x0134,0x0000
};

const uint16_t Data_Audio01[] PROGMEM = {
	0x0045,0x0184,0x0005,0x0134,0x0130,0x012c,0x0128,0x0124,0x0220,0x011c,0x0000
};

const uint16_t Data_Audio02[] PROGMEM = {
	0x0005,0x014c,0x0045,0x0174,0x0164,0x0160,0x0134,0x0120,0x0005,0x010c,0x0104,0x0108,0x05fc,0x0108,0x0000
};

const uint16_t Data_Audio03[] PROGMEM = {
	0x0005,0x0150,0x028c,0x0194,0x01fc,0x0120,0x02fc,0x0118,0x0114,0x01fc,0x0210,0x010c,0x0000
};

const uint16_t Data_Audio04[] PROGMEM = {
	0x0005,0x013c,0x0470,0x02fc,0x0194,0x01fc,0x0294,0x01fc,0x0294,0x0000
};

#define NUM_AUDIO_PATTERNS 5
const uintptr_t* Data_AudioPatterns[NUM_AUDIO_PATTERNS] PROGMEM = {
	Data_Audio00,
	Data_Audio01,
	Data_Audio02,
	Data_Audio03,
	Data_Audio04,
};
