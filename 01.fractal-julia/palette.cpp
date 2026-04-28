/*
 * palette.cpp
#0D0887
#2F0595
#4F03A1
#6901A6
#830AA4
#9C179E
#B02A90
#C23D81
#D35170
#E06561
#ED7953
#F59044
#FBA836
#FDC12A
#F8DD24
#F0F921

--------------

#00224E
#152F61
#293C71
#354775
#425276
#505C72
#5D676E
#6A726D
#787D6C
#86886D
#94936D
#A39F6C
#B3AC6A
#C3B868
#E0CF52
#FEE838
 */

#include "palette.h"

uint32_t bswap32(uint32_t x)
{
    return (x & 0x000000FF << 24) |
           (x & 0x0000FF00 << 8) |
           (x & 0x00FF0000 >> 8) |
           (x & 0xFF000000 >> 24);
}

std::vector<uint32_t> color_ramp = {
    bswap32(0xF0F921FF),
    bswap32(0xF8DD24FF),
    bswap32(0xFDC12AFF),
    bswap32(0xFBA836FF),
    bswap32(0xF59044FF),
    bswap32(0xED7953FF),
    bswap32(0xE06561FF),
    bswap32(0xD35170FF),
    bswap32(0xC23D81FF),
    bswap32(0xB02A90FF),
    bswap32(0x9C179EFF),
    bswap32(0x830AA4FF),
    bswap32(0x6901A6FF),
    bswap32(0x4F03A1FF),
    bswap32(0x2F0595FF),
    bswap32(0x0D0887FF)};

std::vector<uint32_t> color_ramp2 = {
    bswap32(0x00224EFF),
    bswap32(0x152F61FF),
    bswap32(0x293C71FF),
    bswap32(0x354775FF),
    bswap32(0x425276FF),
    bswap32(0x505C72FF),
    bswap32(0x5D676EFF),
    bswap32(0x6A726DFF),
    bswap32(0x787D6CFF),
    bswap32(0x86886DFF),
    bswap32(0x94936DFF),
    bswap32(0xA39F6CFF),
    bswap32(0xB3AC6AFF),
    bswap32(0xC3B868FF),
    bswap32(0xE0CF52FF),
    bswap32(0xFEE838FF)
};

