/*
 * palette.cpp
#000004
#160B39
#3B0F70
#57157E
#721F81
#8C2981
#A8327D
#C43C75
#DF4A66
#F1605D
#FA7F5E
#FEA068
#FEC27A
#F5E68C
#FCFDBF
#FFFFFF
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
    bswap32(0x000004FF),
    bswap32(0x160B39FF),
    bswap32(0x3B0F70FF),
    bswap32(0x57157EFF),
    bswap32(0x721F81FF),
    bswap32(0x8C2981FF),
    bswap32(0xA8327DFF),
    bswap32(0xC43C75FF),
    bswap32(0xDF4A66FF),
    bswap32(0xF1605DFF),
    bswap32(0xFA7F5EFF),
    bswap32(0xFEA068FF),
    bswap32(0xFEC27AFF),
    bswap32(0xF5E68CFF),
    bswap32(0xFCFDBFFF),
    bswap32(0xFFFFFFFF)
};
