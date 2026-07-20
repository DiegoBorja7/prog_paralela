#include "palette.h"

/*
#FFFFCC
#FFF7B5
#FFEC9D
#FEE287
#FED570
#FEC05A
#FEAB49
#FD9740
#FD7D37
#FC5B2E
#F53B23
#E81D1A
#D50E1F
#C00125
#A10027
#800026
*/

/*
0x00000400
0x09042711
0x1E0C4522
0x36106333
0x56137C44
0x721E8055
0x8B288266
0xA7317F77
0xCB347488
0xDF476799
0xED5555AA
0xF97D5EBB
0xFCA170CC
0xFEBF84DD
0xFDE5A1EE
0xFCFDBFFF

*/



uint32_t bswap32(uint32_t a){
    return
    ((a & 0x000000FF)<<24) |
    ((a & 0x0000FF00)<< 8) |
    ((a & 0x00FF0000)>> 8) |
    ((a & 0xFF000000)>>24);
};

std::vector<uint32_t> color_ramp = {
    bswap32(0xFFFFCCFF),
    bswap32(0xFFF7B5FF),
    bswap32(0xFFEC9DFF),
    bswap32(0xFEE287FF),
    bswap32(0xFED570FF),
    bswap32(0xFEC05AFF),
    bswap32(0xFEAB49FF),
    bswap32(0xFD9740FF),
    bswap32(0xFD7D37FF),
    bswap32(0xFC5B2EFF),
    bswap32(0xF53B23FF),
    bswap32(0xE81D1AFF),
    bswap32(0xD50E1FFF),
    bswap32(0xC00125FF),
    bswap32(0xA10027FF),
    bswap32(0x800026FF)
};

std::vector<uint32_t> color_ramp2 = {
    bswap32(0x00000400),
    bswap32(0x09042711),
    bswap32(0x1E0C4522),
    bswap32(0x36106333),
    bswap32(0x56137C44),
    bswap32(0x721E8055),
    bswap32(0x8B288266),
    bswap32(0xA7317F77),
    bswap32(0xCB347488),
    bswap32(0xDF476799),
    bswap32(0xED5555AA),
    bswap32(0xF97D5EBB),
    bswap32(0xFCA170CC),
    bswap32(0xFEBF84DD),
    bswap32(0xFDE5A1EE),
    bswap32(0xFCFDBFFF)
};

std::vector<uint32_t> color_ramp3 = {
    bswap32(0x000000FF), // Black
    bswap32(0x000033FF),
    bswap32(0x000066FF),
    bswap32(0x1B0088FF), // Dark Blue
    bswap32(0x3B00A8FF),
    bswap32(0x5A00C8FF),
    bswap32(0x8A00C8FF), // Purple
    bswap32(0xBA00A8FF),
    bswap32(0xEA0055FF), // Red
    bswap32(0xFF2200FF),
    bswap32(0xFF6600FF), // Orange
    bswap32(0xFFAA00FF), // Yellow
    bswap32(0xFFFF00FF),
    bswap32(0xFFFF55FF),
    bswap32(0xFFFFAAFF),
    bswap32(0xFFFFFFFF)  // White
};

