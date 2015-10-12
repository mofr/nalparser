#pragma once

static unsigned char StartCodePrefix[] = {0x00, 0x00, 0x00, 0x01};
static const int StartCodePrefixLength = 4;

static const char* nalTypes[] = {
        "TRAIL_N",
        "TRAIL_R",
        "TSA_N",
        "TSA_R",
        "STSA_N",
        "STSA_R",
        "RADL_N",
        "RADL_R",
        "RASL_N",
        "RASL_R",
        "RSV_VCL_N10",
        "RSV_VCL_N12",
        "RSV_VCL_N14",
        "RSV_VCL_R11",
        "RSV_VCL_R13",
        "RSV_VCL_R15",
        "BLA_W_LP",
        "BLA_W_RADL",
        "BLA_N_LP",
        "IDR_W_RADL",
        "IDR_N_LP",
        "CRA_NUT",
        "RSV_IRAP_VCL22",
        "RSV_IRAP_VCL23",
        "RSV_VCL24",
        "RSV_VCL25",
        "RSV_VCL26",
        "RSV_VCL28",
        "RSV_VCL29",
        "RSV_VCL30",
        "RSV_VCL31",
        "VPS_NUT",
        "SPS_NUT",
        "PPS_NUT",
        "AUD_NUT",
        "EOS_NUT",
        "EOB_NUT",
        "FD_NUT",
        "PREFIX_SEI_NUT",
        "SUFFIX_SEI_NUT",
        "RSV_NVCL41",
        "RSV_NVCL42",
        "RSV_NVCL43",
        "RSV_NVCL44",
        "RSV_NVCL45",
        "RSV_NVCL46",
        "RSV_NVCL47"
};

inline const char * nalTypeAsString(int type)
{
    if(type < 0 || type > 47)
    {
        return "UNKNOWN";
    }
    else
    {
        return nalTypes[type];
    }
}

struct NalUnit
{
    long offset = 0;
    long size = 0;
    int type = 0;
    bool first = false;
    int elapsedMillis = 0;

    NalUnit()
    { }

    NalUnit(long offset, long size, int type, bool first)
            : offset(offset), type(type), size(size), first(first)
    { }
};