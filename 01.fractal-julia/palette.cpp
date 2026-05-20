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

---------------------

#4477AA
#5299C5
#5FBBE0
#58BEC9
#3DA37E
#228833
#669C3A
#AAB141
#D3AA4E
#E08863
#EE6677
#D35277
#B83D77
#AD4E85
#B485A0
#BBBBBB
 */

#include "palette.h"

uint32_t bswap32(uint32_t x)
{
    return ((x & 0x000000FFu) << 24) |
           ((x & 0x0000FF00u) << 8) |
           ((x & 0x00FF0000u) >> 8) |
           ((x & 0xFF000000u) >> 24);
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
    bswap32(0xFEE838FF)};

std::vector<uint32_t> color_ramp3 = {
    bswap32(0x4477AAFF),
    bswap32(0x5299C5FF),
    bswap32(0x5FBBE0FF),
    bswap32(0x58BEC9FF),
    bswap32(0x3DA37EFF),
    bswap32(0x228833FF),
    bswap32(0x669C3AFF),
    bswap32(0xAAB141FF),
    bswap32(0xD3AA4EFF),
    bswap32(0xE08863FF),
    bswap32(0xEE6677FF),
    bswap32(0xD35277FF),
    bswap32(0xB83D77FF),
    bswap32(0xAD4E85FF),
    bswap32(0xB485A0FF),
    bswap32(0xBBBBBBFF)};
