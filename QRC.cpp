
#include "QRC.hpp"


// make defines to easily access The >>QR code (which I referred to at that time as Module)<< with xy coordinates, TODO make it better
#define MODULE_xy(x, y) (*(uint16_t*)(&(Module[(y) * Dw + ((x) >> 3)])))    // don't use, it is intended only for macros. wonky type code

#define MODULE_get(x, y) ((uint8_t)(MODULE_xy((x), (y)) >> ((x) % 8)))        // get a byte at xy coordinates (no masking)
#define MODULE_clear(x, y, mask) ((MODULE_xy((x), (y))) &= ~(((unsigned)(mask)) << ((x) % 8))) // clear bits at xy with a mask
#define MODULE_write(x, y, mask) ((MODULE_xy((x), (y))) |= (((unsigned)(mask)) << ((x) % 8)))   // write bits at xy with a mask


/*********************************
 * @category tables of constants *
 * @todo calculate those? how?
 *********************************/

/**
 * @brief QR code encoding limits
 * @example [Version][Mode][EC-Level] 
 */
int const qrlimits[40][3][4] = {
    {{41,34,27,17},{25,20,16,10},{17,14,11,7}},
    {{77,63,48,34},{47,38,29,20},{32,26,20,14}},
    {{127,101,77,58},{77,61,47,35},{53,42,32,24}},
    {{187,149,111,82},{114,90,67,50},{78,62,46,34}},
    {{255,202,144,106},{154,122,87,64},{106,84,60,44}},
    {{322,255,178,139},{195,154,108,84},{134,106,74,58}},
    {{370,293,207,154},{224,178,125,93},{154,122,86,64}},
    {{461,365,259,202},{279,221,157,122},{192,152,108,84}},
    {{552,432,312,235},{335,262,189,143},{230,180,130,98}},
    {{652,513,364,288},{395,311,221,174},{271,213,151,119}},
    {{772,604,427,331},{468,366,259,200},{321,251,177,137}},
    {{883,691,489,374},{535,419,296,227},{367,287,203,155}},
    {{1022,796,580,427},{619,483,352,259},{425,331,241,177}},
    {{1101,871,621,468},{667,528,376,283},{458,362,258,194}},
    {{1250,991,703,530},{758,600,426,321},{520,412,292,220}},
    {{1408,1082,775,602},{854,656,470,365},{586,450,322,250}},
    {{1548,1212,876,674},{938,734,531,408},{644,504,364,280}},
    {{1725,1346,948,746},{1046,816,574,452},{718,560,394,310}},
    {{1903,1500,1063,813},{1153,909,644,493},{792,624,442,338}},
    {{2061,1600,1159,919},{1249,970,702,557},{858,666,482,382}},
    {{2232,1708,1224,969},{1352,1035,742,587},{929,711,509,403}},
    {{2409,1872,1358,1056},{1460,1134,823,640},{1003,779,565,439}},
    {{2620,2059,1468,1108},{1588,1248,890,672},{1091,857,611,461}},
    {{2812,2188,1588,1228},{1704,1326,963,744},{1171,911,661,511}},
    {{3057,2395,1718,1286},{1853,1451,1041,779},{1273,997,715,535}},
    {{3283,2544,1804,1425},{1990,1542,1094,864},{1367,1059,751,593}},
    {{3517,2701,1933,1501},{2132,1637,1172,910},{1465,1125,805,625}},
    {{3669,2857,2085,1581},{2223,1732,1263,958},{1528,1190,868,658}},
    {{3909,3035,2181,1677},{2369,1839,1322,1016},{1628,1264,908,698}},
    {{4158,3289,2358,1782},{2520,1994,1429,1080},{1732,1370,982,742}},
    {{4417,3486,2473,1897},{2677,2113,1499,1150},{1840,1452,1030,790}},
    {{4686,3693,2670,2022},{2840,2238,1618,1226},{1952,1538,1112,842}},
    {{4965,3909,2805,2157},{3009,2369,1700,1307},{2068,1628,1168,898}},
    {{5253,4134,2949,2301},{3183,2506,1787,1394},{2188,1722,1228,958}},
    {{5529,4343,3081,2361},{3351,2632,1867,1431},{2303,1809,1283,983}},
    {{5836,4588,3244,2524},{3537,2780,1966,1530},{2431,1911,1351,1051}},
    {{6153,4775,3417,2625},{3729,2894,2071,1591},{2563,1989,1423,1093}},
    {{6479,5039,3599,2735},{3927,3054,2181,1658},{2699,2099,1499,1139}},
    {{6743,5313,3791,2927},{4087,3220,2298,1774},{2809,2213,1579,1219}},
    {{7089,5596,3993,3057},{4296,3391,2420,1852},{2953,2331,1663,1273}}
};

/**
 * @brief error correction codewords and block information
 * @example [Version][EC-Level][qrecbi_t] 
 * @see https://www.thonky.com/qr-code-tutorial/error-correction-table
 */
int const qrECBI[40][4][5] = {
    {{7,1,19,0,0},{10,1,16,0,0},{13,1,13,0,0},{17,1,9,0,0}},
    {{10,1,34,0,0},{16,1,28,0,0},{22,1,22,0,0},{28,1,16,0,0}},
    {{15,1,55,0,0},{26,1,44,0,0},{18,2,17,0,0},{22,2,13,0,0}},
    {{20,1,80,0,0},{18,2,32,0,0},{26,2,24,0,0},{16,4,9,0,0}},
    {{26,1,108,0,0},{24,2,43,0,0},{18,2,15,2,16},{22,2,11,2,12}},
    {{18,2,68,0,0},{16,4,27,0,0},{24,4,19,0,0},{28,4,15,0,0}},
    {{20,2,78,0,0},{18,4,31,0,0},{18,2,14,4,15},{26,4,13,1,14}},
    {{24,2,97,0,0},{22,2,38,2,39},{22,4,18,2,19},{26,4,14,2,15}},
    {{30,2,116,0,0},{22,3,36,2,37},{20,4,16,4,17},{24,4,12,4,13}},
    {{18,2,68,2,69},{26,4,43,1,44},{24,6,19,2,20},{28,6,15,2,16}},
    {{20,4,81,0,0},{30,1,50,4,51},{28,4,22,4,23},{24,3,12,8,13}},
    {{24,2,92,2,93},{22,6,36,2,37},{26,4,20,6,21},{28,7,14,4,15}},
    {{26,4,107,0,0},{22,8,37,1,38},{24,8,20,4,21},{22,12,11,4,12}},
    {{30,3,115,1,116},{24,4,40,5,41},{20,11,16,5,17},{24,11,12,5,13}},
    {{22,5,87,1,88},{24,5,41,5,42},{30,5,24,7,25},{24,11,12,7,13}},
    {{24,5,98,1,99},{28,7,45,3,46},{24,15,19,2,20},{30,3,15,13,16}},
    {{28,1,107,5,108},{28,10,46,1,47},{28,1,22,15,23},{28,2,14,17,15}},
    {{30,5,120,1,121},{26,9,43,4,44},{28,17,22,1,23},{28,2,14,19,15}},
    {{28,3,113,4,114},{26,3,44,11,45},{26,17,21,4,22},{26,9,13,16,14}},
    {{28,3,107,5,108},{26,3,41,13,42},{30,15,24,5,25},{28,15,15,10,16}},
    {{28,4,116,4,117},{26,17,42,0,0},{28,17,22,6,23},{30,19,16,6,17}},
    {{28,2,111,7,112},{28,17,46,0,0},{30,7,24,16,25},{24,34,13,0,0}},
    {{30,4,121,5,122},{28,4,47,14,48},{30,11,24,14,25},{30,16,15,14,16}},
    {{30,6,117,4,118},{28,6,45,14,46},{30,11,24,16,25},{30,30,16,2,17}},
    {{26,8,106,4,107},{28,8,47,13,48},{30,7,24,22,25},{30,22,15,13,16}},
    {{28,10,114,2,115},{28,19,46,4,47},{28,28,22,6,23},{30,33,16,4,17}},
    {{30,8,122,4,123},{28,22,45,3,46},{30,8,23,26,24},{30,12,15,28,16}},
    {{30,3,117,10,118},{28,3,45,23,46},{30,4,24,31,25},{30,11,15,31,16}},
    {{30,7,116,7,117},{28,21,45,7,46},{30,1,23,37,24},{30,19,15,26,16}},
    {{30,5,115,10,116},{28,19,47,10,48},{30,15,24,25,25},{30,23,15,25,16}},
    {{30,13,115,3,116},{28,2,46,29,47},{30,42,24,1,25},{30,23,15,28,16}},
    {{30,17,115,0,0},{28,10,46,23,47},{30,10,24,35,25},{30,19,15,35,16}},
    {{30,17,115,1,116},{28,14,46,21,47},{30,29,24,19,25},{30,11,15,46,16}},
    {{30,13,115,6,116},{28,14,46,23,47},{30,44,24,7,25},{30,59,16,1,17}},
    {{30,12,121,7,122},{28,12,47,26,48},{30,39,24,14,25},{30,22,15,41,16}},
    {{30,6,121,14,122},{28,6,47,34,48},{30,46,24,10,25},{30,2,15,64,16}},
    {{30,17,122,4,123},{28,29,46,14,47},{30,49,24,10,25},{30,24,15,46,16}},
    {{30,4,122,18,123},{28,13,46,32,47},{30,48,24,14,25},{30,42,15,32,16}},
    {{30,20,117,4,118},{28,40,47,7,48},{30,43,24,22,25},{30,10,15,67,16}},
    {{30,19,118,6,119},{28,18,47,31,48},{30,34,24,34,25},{30,20,15,61,16}}
};

/**
 * @brief qr alignment column and row table
 * @example [version][index of align pattern]
 * @see https://www.thonky.com/qr-code-tutorial/alignment-pattern-locations
 */
uint8_t const qralignclrw[40][7] =
{
    {0,0,0,0,0,0,0},
    {6,18,0,0,0,0,0},
    {6,22,0,0,0,0,0},
    {6,26,0,0,0,0,0},
    {6,30,0,0,0,0,0},
    {6,34,0,0,0,0,0},
    {6,22,38,0,0,0,0},
    {6,24,42,0,0,0,0},
    {6,26,46,0,0,0,0},
    {6,28,50,0,0,0,0},
    {6,30,54,0,0,0,0},
    {6,32,58,0,0,0,0},
    {6,34,62,0,0,0,0},
    {6,26,46,66,0,0,0},
    {6,26,48,70,0,0,0},
    {6,26,50,74,0,0,0},
    {6,30,54,78,0,0,0},
    {6,30,56,82,0,0,0},
    {6,30,58,86,0,0,0},
    {6,34,62,90,0,0,0},
    {6,28,50,72,94,0,0},
    {6,26,50,74,98,0,0},
    {6,30,54,78,102,0,0},
    {6,28,54,80,106,0,0},
    {6,32,58,84,110,0,0},
    {6,30,58,86,114,0,0},
    {6,34,62,90,118,0,0},
    {6,26,50,74,98,122,0},
    {6,30,54,78,102,126,0},
    {6,26,52,78,104,130,0},
    {6,30,56,82,108,134,0},
    {6,34,60,86,112,138,0},
    {6,30,58,86,114,142,0},
    {6,34,62,90,118,146,0},
    {6,30,54,78,102,126,150},
    {6,24,50,76,102,128,154},
    {6,28,54,80,106,132,158},
    {6,32,58,84,110,136,162},
    {6,26,54,82,110,138,166},
    {6,30,58,86,114,142,170},
};

/** 
 * @brief format information
 * @example [EC-Level][Mask-Pattern]
 * @see https://www.thonky.com/qr-code-tutorial/format-version-tables
 */
uint16_t const qrfmtinfo[4][8] =
{
    {   // EC-Level 0
        0b111011111000100,
        0b111001011110011,
        0b111110110101010,
        0b111100010011101,
        0b110011000101111,
        0b110001100011000,
        0b110110001000001,
        0b110100101110110,
    },
    {   // EC-Level 1
        0b101010000010010,
        0b101000100100101,
        0b101111001111100,
        0b101101101001011,
        0b100010111111001,
        0b100000011001110,
        0b100111110010111,
        0b100101010100000,
    },
    {   // EC-Level 2
        0b011010101011111,
        0b011000001101000,
        0b011111100110001,
        0b011101000000110,
        0b010010010110100,
        0b010000110000011,
        0b010111011011010,
        0b010101111101101,
    },
    {   // EC-Level 3
        0b001011010001001,
        0b001001110111110,
        0b001110011100111,
        0b001100111010000,
        0b000011101100010,
        0b000001001010101,
        0b000110100001100,
        0b000100000111011,
    },
};

/** 
 * @brief version information
 * @example [Version - 6] with Version within 6...39
 * @see refer https://www.thonky.com/qr-code-tutorial/format-version-tables
 */
uint32_t const qrverinfo[] =
{
    0b000111110010010100,
    0b001000010110111100,
    0b001001101010011000,
    0b001010010011010010,
    0b001011101111110110,
    0b001100011101100010,
    0b001101100001000110,
    0b001110011000001100,
    0b001111100100101000,
    0b010000101101111000,
    0b010001010001011100,
    0b010010101000010100,
    0b010011010100110000,
    0b010100100110100100,
    0b010101011010000000,
    0b010110100011001000,
    0b010111011111101100,
    0b011000111011000100,
    0b011001000111100000,
    0b011010111110101000,
    0b011011000010001100,
    0b011100110000011000,
    0b011101001100111100,
    0b011110110101110100,
    0b011111001001010000,
    0b100000100111010000,
    0b100001011011110000,
    0b100010100010111000,
    0b100011011110011000,
    0b100100101100001000,
    0b100101010000101000,
    0b100110101001100000,
    0b100111010101000000,
    0b101000110001101000,
};

/** 
 * @brief finder pattern
 * @example [row]
 */
uint16_t const qrfinder[] =
{
    0b0000000111111111,
    0b0000000100000001,
    0b0000000101111101,
    0b0000000101000101,
    0b0000000101000101,
    0b0000000101000101,
    0b0000000101111101,
    0b0000000100000001,
    0b0000000111111111,
};

/** 
 * @brief alignment pattern
 * @example [row]
 */
uint8_t const qralignment[] =
{
    0b00000000,
    0b00001110,
    0b00001010,
    0b00001110,
    0b00000000,
};

/** @see https://www.thonky.com/qr-code-tutorial/log-antilog-table */
int const qrLog[256] = {1,2,4,8,16,32,64,128,29,58,116,232,205,135,19,38,76,152,45,90,180,117,234,201,143,3,6,12,24,48,96,192,157,39,78,156,37,74,148,53,106,212,181,119,238,193,159,35,70,140,5,10,20,40,80,160,93,186,105,210,185,111,222,161,95,190,97,194,153,47,94,188,101,202,137,15,30,60,120,240,253,231,211,187,107,214,177,127,254,225,223,163,91,182,113,226,217,175,67,134,17,34,68,136,13,26,52,104,208,189,103,206,129,31,62,124,248,237,199,147,59,118,236,197,151,51,102,204,133,23,46,92,184,109,218,169,79,158,33,66,132,21,42,84,168,77,154,41,82,164,85,170,73,146,57,114,228,213,183,115,230,209,191,99,198,145,63,126,252,229,215,179,123,246,241,255,227,219,171,75,150,49,98,196,149,55,110,220,165,87,174,65,130,25,50,100,200,141,7,14,28,56,112,224,221,167,83,166,81,162,89,178,121,242,249,239,195,155,43,86,172,69,138,9,18,36,72,144,61,122,244,245,247,243,251,235,203,139,11,22,44,88,176,125,250,233,207,131,27,54,108,216,173,71,142,1};
/** @see https://www.thonky.com/qr-code-tutorial/log-antilog-table */
int const qrAntilog[256] = {255,0,1,25,2,50,26,198,3,223,51,238,27,104,199,75,4,100,224,14,52,141,239,129,28,193,105,248,200,8,76,113,5,138,101,47,225,36,15,33,53,147,142,218,240,18,130,69,29,181,194,125,106,39,249,185,201,154,9,120,77,228,114,166,6,191,139,98,102,221,48,253,226,152,37,179,16,145,34,136,54,208,148,206,143,150,219,189,241,210,19,92,131,56,70,64,30,66,182,163,195,72,126,110,107,58,40,84,250,133,186,61,202,94,155,159,10,21,121,43,78,212,229,172,115,243,167,87,7,112,192,247,140,128,99,13,103,74,222,237,49,197,254,24,227,165,153,119,38,184,180,124,17,68,146,217,35,32,137,46,55,63,209,91,149,188,207,205,144,135,151,178,220,252,190,97,242,86,211,171,20,42,93,158,132,60,57,83,71,109,65,162,31,45,67,216,183,123,164,118,196,23,73,236,127,12,111,246,108,161,59,82,41,157,85,170,251,96,134,177,187,204,62,90,203,89,95,176,156,169,160,81,11,245,22,235,122,117,44,215,79,174,213,233,230,231,173,232,116,214,244,234,168,80,88,175};

/** 
 * @brief how many bits for the character count indicator for each [Version] and [Mode]
 * @example [Version][Mode] 
 * @see (step 4 the 1st) https://www.thonky.com/qr-code-tutorial/data-encoding 
 */
int const qrcharcount[3][3] = {
    {10, 9, 8},
    {12, 11, 16},
    {14, 13, 16},
};

/********************************
 * @category private class code *
 ********************************/

/**
 * @brief checks if the given character is in the numeric set
 * @return true if that's the case
 */
bool QRC::IsNumeric()
{
    bool check = false;
    if(this->ch >= '0' && this->ch <= '9') check = true;
    return check;
}

/**
 * @brief checks if the given character is in the alphanumerical set
 * @return true if that's the case
 */
bool QRC::IsAlphanumeric()
{
    bool check = false;
    if(this->ch >= '0' && this->ch <= '9') check = true;
    else if(this->ch >= 'A' && this->ch <= 'Z') check = true;
    else if(this->ch == ' ' || this->ch == '$' || this->ch == '%' || this->ch == '*' || this->ch == '+') check = true;
    else if(this->ch == '-' || this->ch == '.' || this->ch == '/' || this->ch == '/') check = true;
    return check;
}

/**
 * @brief checks if the given character is in the alphanumerical set
 * @return true if that's the case
 */
uint8_t QRC::AlphanumericCode(char ch)
{
    if(ch >= '0' && ch <= '9')
    {
        return (ch - '0');
    }
    if(ch >= 'A' && ch <= 'Z')
    {
        return (ch - 'A' + 10);
    }
    switch(ch)
    {
        case ' ': return 36;
        case '$': return 37;
        case '%': return 38;
        case '*': return 39;
        case '+': return 40;
        case '-': return 41;
        case '.': return 42;
        case '/': return 43;
        case ':': return 44;
        default : return 0;
    }
}

/**
 * @brief checks if the given character is in the byte set
 * @return true if that's the case
 */
bool QRC::IsByte()
{
    bool check = false;
    if(this->ch >= 0x20 && this->ch <= 0x7E) check = true;
    else if(this->ch >= 0xA0 && this->ch <= 0xFF) check = true;
    return check;
}

/**
 * @brief append a number to the raw thing
 * @param Raw - the raw thing
 * @param Number - the number to append
 * @param Pad - the padding before the number
 */
void QRC::AppendNumber(raw_t *Raw, int Number, int Pad)
{
    DEBUG(int PadD = Pad);
    DEBUG(int NumberD = Number);
    DEBUG(printf("Append this: "));
    
    // determine bitlength of number
    int NumberBL = Number;
    int BitLength = 0;
    while(NumberBL > 0)
    {
        BitLength++;
        NumberBL >>= 1;
    }
    // pad first
    while(Pad > BitLength)
    {
        // check if buffer has to be resized
        if(!(Raw->Offset % 8))
        {
            Raw->Data.resize(Raw->Data.size() + 1);
        }
        // add digit 0
        (*Raw).Data[Raw->Data.size() - 1] &= ~(0x01 << (7 - Raw->Offset % 8));              // clear bit
        DEBUG(printf("0"));//, (Number & 0x01)));
        Raw->Offset++;
        Pad--;
    }
    // create binary number and append
    while(BitLength > 0)
    {
        BitLength--;
        // check if buffer has to be resized
        if(!(Raw->Offset % 8))
        {
            Raw->Data.resize(Raw->Data.size() + 1);
        }
        // add digit
        Raw->Data[Raw->Data.size() - 1] &= ~(1 << (7 - Raw->Offset % 8));   // clear bit
        Raw->Data[Raw->Data.size() - 1] |= (((Number >> BitLength) & 0x01) << (7 - Raw->Offset % 8));     // set bit
        DEBUG(printf("%d", ((Number >> BitLength) & 0x01)));
        //Number <<= 1;
        Raw->Offset++;
    }
    DEBUG(if(PadD == 8))
    DEBUG(printf(" - 0x%02X [%1c]", NumberD, NumberD));
    DEBUG(printf("\n"));
}

/*  func    EC_Coding
 *  desc    create error correction codewords
 *          https://www.thonky.com/qr-code-tutorial/error-correction-coding
 */
void QRC::EC_Coding(std::vector<uint8_t> *Data, qr_Nb_t Nb)
{
    // split into groups
    // first group
    for(int i = 0; i < Nb.B1; i++)
    {
        for(int j = 0; j < Nb.Data_CW_B1; j++)
        {
            
        }
    }
    // second group
    for(int i = 0; i < Nb.B2; i++)
    {
        for(int j = 0; j < Nb.Data_CW_B2; j++)
        {
            
        }
    }
}

/**
 * @brief creates generator polynomials
 * @param Polynomials - pointer to polynomials
 * @param Count - count of polys
 * @see https://www.thonky.com/qr-code-tutorial/error-correction-coding
 * @note 1st array element is x^0
 *       2nd array element is x^1
 *       3rd array element is x^2
 *       4th array element is x^3
 *       ...(and so on)
 */
void QRC::CreateGeneratorPoly(std::vector<uint8_t> *Polynomials, int Count)
{
    if(Count <= 1) return;   // makes no sense to calculate a "zero" or "one" generator polynomial
    
    // setting up
    int Annex = 0;
    (*Polynomials).resize(Count + 1);
    (*Polynomials)[0] = 0;  // set first element to 0
    
    // the main thingy
    for(int i = 0; i < Count; i++)
    {
        // cleaning the polynomials array from after initializing
        (*Polynomials)[i + 1] = 0;
        
        // combine like terms
        for(int j = i; j > 0; j--)
        {
            #define X (((uint16_t)qrLog[(*Polynomials)[j - 1]]) ^ ((uint16_t)qrLog[((*Polynomials)[j] + Annex) % 255]))
            (*Polynomials)[j] = (uint8_t)((X % 256) + (X >> 8));
            #undef X
        }
        
        if(i > 0)   // TODO maybe replace this with a if i == 0 continue?
        {
            (*Polynomials)[0] = ((*Polynomials)[0] + Annex) % 255;
        }
        
        // gf256 back => to alpha notation
        for(int j = 1; j < i + 1; j++)
        {
            (*Polynomials)[j] = qrAntilog[(*Polynomials)[j]];
        }
        
        // next annex
        Annex++;
    }
}

/**
 * @brief idek anymore
 * @param ECCGB
 * @param Message
 * @param Polynomials
 * @param Mseek
 * @param NbD
 * @param NbB
 * @param NbCW
 */
void QRC::CreateECCGB(std::vector<std::vector<std::vector<uint8_t>>> *ECCGB, \
                      std::vector<uint8_t> Message, \
                      std::vector<uint8_t> Polynomials, \
                      int *Mseek, \
                      int NbD, \
                      int NbB, \
                      int NbCW)    // create ecc blocks & groups
{
    int Mlen = Message.size();
    if(NbB)
    {
        int size = (int)ECCGB->size();
        // allocate for group
        ECCGB->resize(size + 1);
        DEBUG(printf("...for group %d...\n", size + 1));
        (*ECCGB)[size].resize(NbB);
        for(int i = 0; i < NbB; i++)
        {
            DEBUG(printf("data codewords for block %d: ", i));
            // allocate for blocks
            (*ECCGB)[size][i].resize(NbD);
            for(int j = 0; j < NbD; j++)
            {
                // copy message data
                if(*Mseek < Mlen)
                {
                    DEBUG(printf("%3d,", Message[*Mseek]));
                    (*ECCGB)[size][i][j] = Message[(*Mseek)++];
                }
                else
                {
                    (*ECCGB)[size][i][j] = 0;
                }
            }
            DEBUG(printf("\b \n"));
        }
        
        QRC::CreateECCG(&(*ECCGB)[size], Polynomials, NbB, NbCW);
    }
}

/**
 * @brief idek anymore
 * @param ECCG
 * @param Polynomials
 * @param NbB
 * @param NbCW
 */
void QRC::CreateECCG(std::vector<std::vector<uint8_t>> *ECCG, std::vector<uint8_t> Polynomials, int NbB, int NbCW)
{
    for(int i = 0; i < NbB; i++)
    {
        DEBUG(printf("...for block %d/%d:\n", i + 1, NbB));
        QRC::CreateECC(&(*ECCG)[i], Polynomials, NbCW);
    }
}

/**
 * @brief idek anymore
 * @param ECC
 * @param Polynomials
 * @param NbCW
 */
void QRC::CreateECC(std::vector<uint8_t> *ECC, std::vector<uint8_t> Polynomials, int NbCW)
{
    // now it is possible to perform the repeated division steps
    int Steps = (int)ECC->size();
    for(int i = 0; i < Steps; )
    {
        DEBUG(printf("%2d/%2d: multiply with alpha %3d : ", i + 1, Steps, qrAntilog[(*ECC)[0]]));
        uint8_t XORLead = (*ECC)[0];
        // step a
        DEBUG(for(int j = NbCW; j + 1 > 0; j--))
        DEBUG({)
            // alpha notation
            DEBUG(printf("%3d,", (Polynomials[j] + qrAntilog[XORLead]) % 255));   //=> done in step b
        DEBUG(})
        DEBUG(printf("\b \n"));
        // step b
        DEBUG(printf("%2d/%2d: XOR                     : ", i + 1, Steps));
        int AddN = NbCW + 1 - (int)ECC->size();
        if(AddN > 0)
        {
            ECC->insert(ECC->end(), AddN, 0);
        }
        for(int j = 0; j < (int)ECC->size(); j++)
//        for(int j = 0; j < NbCW; j++)
        {
            // integer notation
            if(j <= NbCW)
            {
                int k = NbCW - j;
                (*ECC)[j] ^= qrLog[(Polynomials[k] + qrAntilog[XORLead]) % 255];
            }
            DEBUG(if(j))
            DEBUG(printf("%3d,", (*ECC)[j]));
        }
        DEBUG(printf("\b "));
        // discard lead 0 term TODO what if all entries are 0?
        DEBUG(int dcd = 0;)
        while(!(*ECC)[0] && i < Steps)  // IMPORTANT TODO make sure that we don't exceed Steps with this here???
        {
            DEBUG(dcd++;)
            i++;
            ECC->erase(ECC->begin());
            if((int)ECC->size() <= NbCW)
            {
                ECC->push_back(0);
            }
        }
        DEBUG(printf(" (dcd : %d)\n", dcd));
    }
    // delete last element which is a 0
    ECC->erase(ECC->end() - 1);
}

/**
 * @brief create message polynomial
 * @param Message - message
 * @param Polynomials - polys
 * @param Nb - number of ...
 * @todo rename the function to its actual doing
 */
std::vector<std::vector<std::vector<uint8_t>>> QRC::CreateMessagePoly(std::vector<uint8_t> Message, std::vector<uint8_t> Polynomials, qr_Nb_t Nb)
{
    DEBUG(printf("Creating Message Polynomial... (below)\n"));
    std::vector<std::vector<std::vector<uint8_t>>> XORResult;
    
    int Mseek = 0;
    
    // create blocks for ECC
    this->CreateECCGB(&XORResult, Message, Polynomials, &Mseek, Nb.Data_CW_B1, Nb.B1, Nb.EC_CW_Per_Block);
    this->CreateECCGB(&XORResult, Message, Polynomials, &Mseek, Nb.Data_CW_B2, Nb.B2, Nb.EC_CW_Per_Block);
    Mseek = 0;
    
    return XORResult;
}

raw_t QRC::CreateFinal(raw_t Raw, std::vector<std::vector<std::vector<uint8_t>>> ECCCW, qr_Nb_t Nb, int Version)
{
    DEBUG(printf("up next: create final message\n"));
    raw_t Final;
    Final.Offset = 0;
    
    // collect information
    int Groups = ECCCW.size();
    std::vector<int> Blocks;
    Blocks.resize(Groups);
    for(int i = 0; i < Groups; i++)
    {
        Blocks[i] = ECCCW[i].size();
    }
    int ECCW = Nb.EC_CW_Per_Block;  // error correction codewords
    
    // interleave the blocks -> data codewords
    DEBUG(printf("interleave the data codewords:\n"));
    for(int i = 0; (i < Nb.Data_CW_B1) || (i < Nb.Data_CW_B2); i++) // nasty
    {
        for(int j = 0; j < Groups; j++)
        {
            for(int k = 0; k < Blocks[j]; k++)
            {
                if((j == 0 && (i < Nb.Data_CW_B1)) || (j == 1 && i < Nb.Data_CW_B2))  // nasty shiz
                {
                    // ultra nasty line below
                    int Index =
                        (i) +                                       // add index of codeword offset
                        (j == 0 ? k * Nb.Data_CW_B1 : 0) +          // add block offset within group 0
                        (j == 1 ? k * Nb.Data_CW_B2 : 0) +          // add block offset within group 1
                        (j == 1 ? Blocks[0] * Nb.Data_CW_B1 : 0);   // add group 0 offset for group 1
                    
                    DEBUG(printf("G:%d,B:%2d,CW:%2d : ", j, k, i));
                    QRC::AppendNumber(&Final, Raw.Data[Index], 8);
                }
            }
        }
    }
    // interleave the blocks -> error correction codewords
    DEBUG(printf("interleave the error correction codewords:\n"));
    for(int i = 0; i < ECCW; i++)
    {
        for(int j = 0; j < Groups; j++)
        {
            for(int k = 0; k < Blocks[j]; k++)
            {
                DEBUG(printf("G:%d,B:%2d,CW:%2d : ", j, k, i));
                QRC::AppendNumber(&Final, ECCCW[j][k][i], 8);
            }
        }
    }
    
    // add remainder bits if necessary
    DEBUG(printf("version-specific remainder bits:\n"));
    // Version = 0 => is actually version 1
    if(Version >= 1 && Version <= 5)    // pad 7
    {
        QRC::AppendNumber(&Final, 0, 7);
    }
    else if((Version >= 13 && Version <= 19) || (Version >= 27 && Version <= 33))   // pad 3
    {
        QRC::AppendNumber(&Final, 0, 3);
    }
    else if(Version >= 20 && Version <= 26) // pad 4
    {
        QRC::AppendNumber(&Final, 0, 4);
    }
    DEBUG(else)
    DEBUG(printf("not needed\n"));
    
    DEBUG(printf("final msg sequence (hex): "));
    DEBUG(for(int i = 0; i < Nb.B1 * Nb.Data_CW_B1 + Nb.B2 * Nb.Data_CW_B2; i++))
    DEBUG(printf("%02X ", Final.Data[i]));
    DEBUG(printf("\n"));
    DEBUG(printf("final cw sequence (hex): "));
    DEBUG(for(int i = Nb.B1 * Nb.Data_CW_B1 + Nb.B2 * Nb.Data_CW_B2; i < Final.Data.size(); i++))
    DEBUG(printf("%02X ", Final.Data[i]));
    DEBUG(printf("\n"));
    
    return Final;
}

/**
 * @brief creates a qr code module with finder and the basic stuff mask (without the actual finders and without the data)
 * @param Version - 0 = version 1, so 39 = version 40
 * @return the created QR code
 * @note it is a 2d-array in a 1d-array
 * @note top left is 0, bottom right is width*height
 */
std::vector<uint8_t> QRC::CreateModule(int Version)
{
    // allocate memory
    int Dr = Version * 4 + 21;      // dimensions (raw, one "pixel")
    int Dw = ((Dr - 1) >> 3) + 1;   // dimension (width, bytes)
    int Dh = Dr;                    // dimensions (height)
    std::vector<uint8_t> Module;
    Module.resize(Dh * Dw);    // shr3 is the same as divide by 8
    
    // place timing patterns
    for(int i = 0; i < Dh; i++)
    {
        MODULE_write(6, i, ((i + 1) % 2) & 0b1);
        MODULE_write(i, 6, ((i + 1) % 2) & 0b1);
    }
    
    // place finders
    for(int i = 0; i < 8; i++)  // TODO magical number 7 (and maybe also below)
    {
        // top left
        MODULE_clear(0, i, 0xFF);
        MODULE_write(0, i, ~(qrfinder[i + 1] >> 1) & 0xFF);
        
        // top right
        MODULE_clear(Dr - 8, i, 0xFF);
        MODULE_write(Dr - 8, i, ~qrfinder[i + 1] & 0xFF);
        
        // bottom left
        MODULE_clear(0, Dh - 8 + i, 0xFF);
        MODULE_write(0, Dh - 8 + i, ~(qrfinder[i] >> 1) & 0xFF);
    }
    
    // place alignment paterns
    for(int i = 0; i < 7; i++)  // TODO magical number 7 (array length of qralignclrw) and also below
    {
        int cx = qralignclrw[Version][i];   // coordinate x
        if(cx == 0) continue;               // is it an allowed position? (zero means no)
        
        for(int j = 0; j < 7; j++)
        {
            int cy = qralignclrw[Version][j];   // coordinate y
            if(cy == 0) continue;               // is it an allowed position? (zero means no)
            
            // current position is not one of where the finders are?
            if(cx == 6 && cy == 6) continue;        // top left
            if(cx == 6 && cy == Dh - 7) continue;   // bottom left
            if(cx == Dr - 7 && cy == 6) continue;   // top right
            
            // place finder
            for(int k = 0; k < 5; k++)  // TODO magical number 5 (and maybe also below)
            {
                MODULE_write(cx - 2, cy - 2 + k, ~qralignment[k] & 0x1F);
            }
        }
    }
    
    // write dark module
    MODULE_write(8, Dh - 8, 0b1);
    
    // fill whole thing
    for(int i = 0; i < Dh; i++)
    {
        for(int j = 0; j < Dh; j++)
        {
            if(QRC::isPlacementLegal(Version, j, i))
            {
                MODULE_clear(i, j, 0b1);
            }
        }
    }
    
    return Module;
}

void display(std::vector<uint8_t> Module, int Version)
{
    int Dr = Version * 4 + 21;      // dimensions (raw, one "pixel")
    int Dw = ((Dr - 1) >> 3) + 1;   // dimension (width, bytes)
    
    // TEMPORARY: display qr code
    printf("%4c ", ' ');
    for(int j = 0; j < Dr; j++) printf("%2d", j % 100);
    printf("\n");
    printf("%4c ", ' ');
    for(int j = 0; j < Dr; j++) printf("[]");
    printf("\n");
    printf("%3c%c%c", ' ', 219, 219);
    for(int j = 0; j < Dr + 1; j++) printf("%c%c", 219, 219);
    printf("\n");
    for(int i = 0; i < Dr; i++)
    {
        printf("%3d%c%c", i, 219, 219);
        for(int j = 0; j < Dr; j++)
        {
            #define X ((MODULE_get(j, i) & 0b1) == 1 ? ' ' : 219)
            printf("%c%c", X, X);
            #undef X
        }
        printf("%c%c|\n", 219, 219);
    }
    printf("%3c%c%c", ' ', 219, 219);
    for(int j = 0; j < Dr + 1; j++) printf("%c%c", 219, 219);
    printf("\n");
    printf("%4d\b  ", 0);
    for(int j = 0; j < Dr; j++) printf("[]");
    printf("\n");
}

/**
 * @brief place final data in module
 * @param Module - QR code
 * @param Final - final data
 * @param Version - must start from 0 up to 39
 * @return 
 */
std::vector<uint8_t> QRC::PlaceInModule(std::vector<uint8_t> Module, raw_t Final, int Version)
{
    
    int Dr = Version * 4 + 21;      // dimensions (raw, one "pixel")
    int Dw = ((Dr - 1) >> 3) + 1;   // dimension (width, bytes)
    int FinalLen = Final.Data.size() * 8;
    
    int i = 0;
    int Direction = -1;  // 1 == down, -1 == up
    
    // start from the bottom to left
    for(int x = Dr - 1; x + 1 > 0; x -= 2)
    {
        // the vertical timing pattern is an exception to the rule
        if(x == 6)
        {
            x--; 
        }
        
        // upwards / downwards
        for(int y = (Direction == 1) ? (0) : (Dr - 1); (Direction == 1) ? (y < Dr) : (y + 1 > 0); y += Direction)
        {
            if(QRC::isPlacementLegal(Version, x, y))
            {
                MODULE_clear(x, y, 0b1);
                if(i >> 3 < FinalLen)
                {
                    MODULE_write(x, y, (Final.Data[i >> 3] >> (7 - i % 8)) & 0b1);  // "right"
                }
                else
                {
                    printf("f");    // TODO remove this
                }
                i++;
            }
            if(QRC::isPlacementLegal(Version, x - 1, y))
            {
                MODULE_clear(x - 1, y, 0b1);
                if(i >> 3 < FinalLen)
                {
                    MODULE_write(x - 1, y, (Final.Data[i >> 3] >> (7 - i % 8)) & 0b1);  // "left"
                }
                else
                {
                    printf("f");    // TODO remove this
                }
                i++;
            }
        }
        Direction *= -1;
    }
    
    return Module;
}

/**
 * @brief place format and version information in module
 * @param Module - QR code
 * @param ECLevel - error correction level
 * @param Mask - number 0...7
 * @param Version - from 0 to 39
 * @return the new QR code
 */
std::vector<uint8_t> QRC::PlaceFmtVer(std::vector<uint8_t> Module, int ECLevel, int Mask, int Version)
{
    int Dr = Version * 4 + 21;      // dimensions (raw, one "pixel")
    int Dw = ((Dr - 1) >> 3) + 1;   // dimension (width, bytes)
    
    // *** format information ***
    
    // TODO make sure that Mask and ECLevel do not exceed the max values... in general and everywhere
    uint16_t Fmt = qrfmtinfo[ECLevel][Mask];
    
    // TODO magical numbers everywhere below
    // next to bottom left finder
    for(int i = 0; i < 7; i++)
    {
        MODULE_clear(8, Dr - i - 1, 0b1);
        MODULE_write(8, Dr - i - 1, (Fmt >> (14 - i)) & 0b1);
    }
    // below top right finder
    for(int i = 0; i < 8; i++)
    {
        MODULE_clear(Dr - i - 1, 8, 0b1);
        MODULE_write(Dr - i - 1, 8, (Fmt >> (i)) & 0b1);
    }
    // around top left finder
    for(int i = 0; i < 8; i++)
    {
        int j = i;      // the actual position
        if(i >= 6) j++; // leave out timing pattern
        
        // below it
        MODULE_clear(j, 8, 0b1);
        MODULE_write(j, 8, (Fmt >> (14 - i)) & 0b1);
        
        // next to it
        MODULE_clear(8, j, 0b1);
        MODULE_write(8, j, (Fmt >> (i)) & 0b1);
    }
    
    // *** version information ***
    
    if(Version >= 6)    // version 7 and larger must include version information
    {
        uint32_t Ver = qrverinfo[Version - 6];
        for(int i = 0; i < 6; i++) // TODO magical number 6 and also more below
        {
            // bottom finder
            int j = 0;
            MODULE_clear(i, Dr - 11, 0b1);
            MODULE_write(i, Dr - 11, (Ver >> (j + i * 3)) & 0b1);
            j++;
            MODULE_clear(i, Dr - 10, 0b1);
            MODULE_write(i, Dr - 10, (Ver >> (j + i * 3)) & 0b1);
            j++;
            MODULE_clear(i, Dr - 9, 0b1);
            MODULE_write(i, Dr - 9, (Ver >> (j + i * 3)) & 0b1);
            
            // right finder
            j = 0;
            MODULE_clear(Dr - 11, i, 0b1);
            MODULE_write(Dr - 11, i, (Ver >> (j + i * 3)) & 0b1);
            j++;
            MODULE_clear(Dr - 10, i, 0b1);
            MODULE_write(Dr - 10, i, (Ver >> (j + i * 3)) & 0b1);
            j++;
            MODULE_clear(Dr - 9, i, 0b1);
            MODULE_write(Dr - 9, i, (Ver >> (j + i * 3)) & 0b1);
        }
    }
    
    return Module;
}


/**
 * @brief applies a mask on a given module
 * @param Module - QR code
 * @param Mask - is 0...7
 * @param Version - starts from 0
 * @return the new module
 * @see https://www.thonky.com/qr-code-tutorial/mask-patterns
 */
std::vector<uint8_t> QRC::ApplyMask(std::vector<uint8_t> Module, int Mask, int Version)
{
    int Dr = Version * 4 + 21;      // dimensions (raw, one "pixel")
    int Dw = ((Dr - 1) >> 3) + 1;   // dimension (width, bytes)
    
    for(int y = 0; y < Dr; y++)
    {
        for(int x = 0; x < Dr; x++)
        {
            bool flip = false;
            switch(Mask)
            {
                case 0: // mask 0:  (row + column) mod 2 == 0
                    if(!((x + y) % 2)) flip = true;
                    break;
                case 1: // mask 1:  (row) mod 2 == 0
                    if(!(y % 2)) flip = true;
                    break;
                case 2: // mask 2:  (column) mod 3 == 0
                    if(!(x % 3)) flip = true;
                    break;
                case 3: // mask 3:  (row + column) mod 3 == 0
                    if(!((x + y) % 3)) flip = true;
                    break;
                case 4: // mask 4:  ( floor(row / 2) + floor(column / 3) ) mod 2 == 0
                    if(!(((y >> 1) + (int)(x / 3)) % 2)) flip = true;
                    break;
                case 5: // mask 5:  ((row * column) mod 2) + ((row * column) mod 3) == 0
                    if(!((x * y) % 2 + ((x * y) % 3))) flip = true;
                    break;
                case 6: // mask 6:  ( ((row * column) mod 2) + ((row * column) mod 3) ) mod 2 == 0
                    if(!(((x * y) % 2 + (x * y) % 3) % 2)) flip = true;
                    break;
                case 7: // mask 7:  ( ((row + column) mod 2) + ((row * column) mod 3) ) mod 2 == 0
                    if(!(((x + y) % 2 + (x * y) % 3) % 2)) flip = true;
                    break;
                default:
                    break;
            }
            if(flip)
            {
                if(QRC::isPlacementLegal(Version, x, y))
                {
                    // flip the bit
                    int old = MODULE_get(x, y);
                    MODULE_clear(x, y, 0b1);
                    MODULE_write(x, y, ~old & 0b1);
                    flip = false;
                }
            }
        }
    }
    return Module;
}

/**
 * @brief determines the mask best suited for the current module
 * @param Module - the QR code
 * @param ECLevel - error correction level
 * @param Version - has to be 0...39
 * @return number of the optimal mask
 * @note Module needs to already have the format and version information applied!
 * @see https://www.thonky.com/qr-code-tutorial/data-masking
 */
int QRC::GetOptimalMask(std::vector<uint8_t> Module, int ECLevel, int Version)
{
    DEBUG(printf("...counting penalty score...\n"));
    int Dr = Version * 4 + 21;      // dimensions (raw, one "pixel")
    int Dw = ((Dr - 1) >> 3) + 1;   // dimension (width, bytes)
    std::vector<uint8_t> Original = Module;
    
    int AllScores[8];  // TODO magic numbers, 8 bc of mask count
    for(int i = 0; i < 8; i++)  // check for each mask TODO magic number
    {
        DEBUG(printf("...for mask %d...\n", i));
        // apply mask
        Module = QRC::ApplyMask(Original, i, Version);
        int Score = 0;  // the total error score
        
        // add format and version information
        Module = QRC::PlaceFmtVer(Module, ECLevel, i, Version);
        
        DEBUG(display(Module, this->Version));
        
        // *** count errors - evalation condition #1 (long lines) ***
        for(int dir = 0; dir < 2; dir++)    // horizontal / vertical
        {
            for(int j = 0; j < Dr; j++)
            {
                int x0 = j * dir;
                int y0 = j * (1 - dir);
                int Track = MODULE_get(x0, y0) & 0b1;
                
//                int Track = (MODULE_get(0, j) & 0b1);
                
                int Consecutive = 1;            // has to start from 1, because of code below
                for(int k = 1; k < Dr; k++)     // don't have to start from 0, because of code below
                {
                    int x = k * (1 - dir) + j * dir;
                    int y = j * (1 - dir) + k * dir;
                    int Here = (MODULE_get(x, y) & 0b1);    // because of code here; track is already set
                    if(Here == Track)
                    {
                        Consecutive++;
                    }
                    else
                    {
                        if(Consecutive >= 5)
                        {
                            Score += (Consecutive - 2);
                        }
                        Track = Here;
                        Consecutive = 1;    // start back at 1
                    }
                }
                if(Consecutive >= 5)
                {
                    Score += (Consecutive - 2);
                }
            }
        }
        DEBUG(int Score1 = Score);
        DEBUG(printf("Score 1-4: %d ", Score1));
        
        // *** count errors - evalation condition #2 (2x2 boxes) ***
        for(int y = 0; y < Dr - 1; y++)
        {
            for(int x = 0; x < Dr - 1; x++)
            {
                int m0 = (MODULE_get(x, y) & 0b11);
                int m1 = (MODULE_get(x, y + 1) & 0b11);
                if(m0 == m1 && ((m0 & 0b1) == (m0 >> 1))) // square found
                {
                    Score += 3;
                }
            }
        }
        DEBUG(int Score2 = Score - Score1);
        DEBUG(printf("%d ", Score2));
        
        // *** count errors - evalation condition #3 (finder like patterns) ***
        int FinderLen = 12; // TODO magic number 12
        for(int dir = 0; dir < 2; dir++)    // 0 = horizontal scan / 1 = vertical scan
        {
            int hits = 0;
            for(int y = 0; y < Dr; y++)
            {
                int x0 = 0; // TODO magic numbers
                int x1 = 7;
                int CDFW = 0; // forward cooldown
                int CDBW = 0; // backward cooldown
                while(x0 < Dr - 8)
                {
                    if(x1 < Dr)
                    {
                        x1++;
                    }
                    if(x1 - x0 > FinderLen || x1 >= Dr)
                    {
                        x0++;
                    }
                    
                    int Delta = x1 - x0;
                    uint16_t Pattern = 0;
                    
                    // get the pattern (FW)
                    for(int i = 0; i < Delta; i++)
                    {
                        Pattern |= ((MODULE_get((x0 + i) * (1 - dir) + y * dir, y * (1 - dir) + dir * (x0 + i)) & 0b1) << i);
                    }
                    
                    int LR = false;
                    int isPatternCheck = false;
                    if(!CDFW)                   // if true, we're currently on the left
                    {
                        if((Delta >= 8 && x0 == 0) || Delta == 12)
                        {
                            isPatternCheck = true;
                        }
                        else if((Delta < 12 && Delta >= 11 && x1 == Dr))
                        {
                            LR = true;
                            isPatternCheck = true;;
                        }
                        
                        if(isPatternCheck)
                        {
                            if(QRC::isFinderPattern(Pattern, Delta, LR))
                            {
                                //TEMPORARY: print binary number
                                int copy = Pattern;
                                int bits = 0;
                                while(copy)
                                {
                                    copy >>= 1;
                                    bits++;
                                }
                                hits++;
                                Score += 40;
                                CDFW = 7;
                            }
                        }

                    }
                    
                    // get the pattern (BW)
                    Pattern = 0;
                    for(int i = 0; i < Delta; i++)
                    {
                        Pattern |= ((MODULE_get((x1 - i - 1) * (1 - dir) + y * dir, y * (1 - dir) + dir * (x1 - i - 1)) & 0b1) << i);
                    }
                    LR = false;
                    isPatternCheck = false;
                    if(!CDBW)             // if true, we're currently on the right side
                    {
                        if((Delta >= 8 && x1 == Dr) || Delta == 12)
                        {
                            isPatternCheck = true;
                            LR = true;
                            Delta *= -1;
                        }
                        else if((Delta < 12 && Delta >= 11 && x0 == 0))
                        {
                            isPatternCheck = true;
                        }
                        
                        if(isPatternCheck)
                        {
                            if(QRC::isFinderPattern(Pattern, Delta, LR))
                            {
                                hits++;
                                Score += 40;
                                CDBW = 7;
                            }
                        }
                    }
                    
                    if(CDFW)
                    {
                        CDFW--;
                    }
                    if(CDBW)
                    {
                        CDBW--;
                    }
                }
            }
        }
        DEBUG(int Score3 = Score - Score1 - Score2);
        DEBUG(printf("%d ", Score3));
        
        // *** count errors - evalation condition #3 (count the modules) ***
        int DarkModules = 0;
        for(int i = 0; i < Dr; i++)
        {
            for(int j = 0; j < Dr; j++)
            {
                if(MODULE_get(j, i) & 0b1)
                {
                    DarkModules++;
                }
            }
        }
        int Percentage = 100 * DarkModules / (Dr * Dr);   // calculate the percentage
        int PreviousMultiple = 5 * (int)(Percentage / 5);    // previous multiple
        int NextMultiple = PreviousMultiple + 5;        // next multiple
        // subtract 50 of both multiples and take the absolute value
        PreviousMultiple = PreviousMultiple >= 50 ? PreviousMultiple - 50 : 50 - PreviousMultiple;
        NextMultiple = NextMultiple >= 50 ? NextMultiple - 50 : 50 - NextMultiple;
        // divide both multiples by five
        PreviousMultiple /= 5;
        NextMultiple /= 5;
        // take the smaller multiple and multiply by 10, add it to the score
        int SmallerMultiple = PreviousMultiple > NextMultiple ? NextMultiple : PreviousMultiple;
        Score += (SmallerMultiple * 10);        
        DEBUG(int Score4 = Score - Score1 - Score2 - Score3);
        DEBUG(printf("%d (second last step is a bit faulty...)\n", Score4));
        DEBUG(printf("final score: %d\n", Score));
        AllScores[i] = Score;
    }
    // get the smallest score
    int Best = 0;
    for(int i = 1; i < 8; i++)  // TODO magic number
    {
        if(AllScores[i] < AllScores[Best])
        {
            Best = i;
        }
    }
    
    // TODO 
    // condition #1 is not correctly (seen it being off by 0...6 points)
    // condition #2 seems to be working (never seen it being off)
    // condition #3 is weird
    // condition #4 seems fine 
    
    DEBUG(printf("...best mask is %d\n", Best));
    return Best;
}


/**
 * @brief checks wheter or not the bits inside the Bits are equal to the finder pattern
 * @param Bits - bit 0 is the first bit (the left bits), bit n [Length] is the last bit (on the right)
 * @param Length - tells from which side of the QR code it is positioned and also
 *                 the direction. positive is left, negative is right
 * @param LR - left = false / right = true
 * @return true if it is a finder pattern
 */
bool QRC::isFinderPattern(uint16_t Bits, int Length, int LR)
{
    uint16_t FinderBW = 0b000010111010; // TODO magic number ?
    uint16_t FinderFW = 0b010111010000; // TODO magic number ?
    uint16_t Finder = 0;                // substitute for finders
    int Offset = 0;
    
    if(Length >= 0)
    {
        Offset = 12 - Length;
        if(Offset > 4)              // Offset should not be greater than 4
        {
            return false;
        }
        if(LR)
        {
            Finder = FinderFW;
        }
        else
        {
            Finder = FinderFW >> Offset;
        }
    }
    else
    {
        Offset = 12 + Length;
        if(Offset > 4)     // Offset should not be greater than 4
        {
            return false;
        }
        if(LR)
        {
            Finder = FinderBW >> 1;
        }
        else
        {
            Finder = FinderBW;
        }
    }
    
    uint16_t Masked = (Bits & ((~((unsigned)(~0x00) << 12)) >> Offset));
    if(Masked == Finder)   // mask the bits for extra safety
    {
        return true;
    }
    
    return false;
}


/**
 * @brief determines wheter a set coordinate (x, y) is not used by either...
 *        ...timing patterns #
 *        ...finder patterns #
 *        ...seperators (of finder patterns) #
 *        ...alignment patterns
 *        ...dark module #
 *        ...reserved format information area #
 *        ...reserved information area #
 * @param Version - the QR code version. starting from 0 up to 39
 * @param x - x coordinate
 * @param y - y coordinate
 * @return if a coordinate is one of those above, returns false. else true
 * @see https://www.thonky.com/qr-code-tutorial/module-placement-matrix
 */
bool QRC::isPlacementLegal(int Version, int x, int y)
{
    #ifdef RANGE
    #error "RANGE has already been defined somewhere, rename it"
    #else
    #define RANGE(v, l, u) ((v) >= (l) && (v) <= (u))
    
    // TODO list
    // TODO remove the RANGE define and everything around it
    // TODO magical "daltaXXX = n" 
    //  \> for deltaFinder see where finders get placed to find out why it is 7
    int const deltaFinder = 7;
    int const deltaTiming = 6;
    int const deltaResFmt = 1;
    int const deltaResVerX = 5; // this is not 6 but 5 and it is intentional, wonky stuff
    int const deltaResVerY = 8;
    
    // alignment pattern
    if(x == deltaTiming) return false;
    if(y == deltaTiming) return false;
    
    // finder pattern - top left & reserved format information area of it
    if(RANGE(x, 0, deltaFinder + deltaResFmt))
    {
        if(RANGE(y, 0, deltaFinder + deltaResFmt))
        {
            return false;
        }
    }
    
    // dimensions (raw, one "pixel")
    int Dr = Version * 4 + 21;
    
    // finder pattern - bottom left & reserved format information area of it
    if(RANGE(x, 0, deltaFinder + deltaResFmt))
    {
        if(RANGE(y, Dr - deltaFinder - 1, Dr))
        {
            return false;
        }
    }
    
    // finder pattern - top right & reserved format information area of it
    if(RANGE(x, Dr - deltaFinder - 1, Dr))
    {
        if(RANGE(y, 0, deltaFinder + deltaResFmt))
        {
            return false;
        }
    }
    
    // dark module
    if(x == 8 && y == Dr - 8) return false;
    
    // version 7 and larger must also reserve version information data
    if(Version >= 6)
    {
        // the one on the bottom
        for(int i = 0; i < 3; i++)
        {
            if(RANGE(y, Dr - 1 - i - deltaResVerY, Dr - 1 - deltaResVerY))
            {
                if(RANGE(x, 0, deltaResVerX))
                {
                    return false;
                }
            }
        }
        
        // the one on the right
        for(int i = 0; i < 3; i++)
        {
            if(RANGE(x, Dr - 1 - i - deltaResVerY, Dr - 1 - deltaResVerY))
            {
                if(RANGE(y, 0, deltaResVerX))
                {
                    return false;
                }
            }
        }
    }
    
    // alignment patterns TODO this is very similar to where these patterns get placed... sus
    for(int i = 0; i < 7; i++)  // TODO magical number 7 (array length of qralignclrw) and also below
    {
        int cx = qralignclrw[Version][i];   // coordinate x
        if(cx == 0) continue;               // is it an allowed position? (zero means no)
        
        for(int j = 0; j < 7; j++)
        {
            int cy = qralignclrw[Version][j];   // coordinate y
            if(cy == 0) continue;               // is it an allowed position? (zero means no)
            
            // current position is not one of where the finders are?
            if(cx == 6 && cy == 6) continue;        // top left
            if(cx == 6 && cy == Dr - 7) continue;   // bottom left
            if(cx == Dr - 7 && cy == 6) continue;   // top right
            
            // check for position
            for(int k = 0; k < 5; k++)  // TODO magical number 5 (and maybe also below)
            {
                if(RANGE(x, cx - 2, cx + 2))    // TODO magical number 2
                {
                    if(RANGE(y, cy - 2, cy - 2 + k))
                    {
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
    #undef RANGE
    #endif
}

/*******************************
 * @category public class code *
 *******************************/

/**
 * @brief QR-code class constructor
 */
QRC::QRC()
{
    // initialize variables
    this->ch = 0;
    this->Version = 0;
    this->Raw.Data.resize(0);
    this->Raw.Offset = 0;
    // init nb of things
    this->Nb.B1 = 0;
    this->Nb.B2 = 0;
    this->Nb.Data_CW_B1 = 0;
    this->Nb.Data_CW_B2 = 0;
    this->Nb.EC_CW_Per_Block = 0;
}

/**
 * @brief the destructor
 * @todo actually code things that belong into here
 */
QRC::~QRC()
{
    
}

/**
 * @brief encode a message
 * @param Message - the message
 * @param EC_Level - a number from 0 to 3. Larger number creates a better error encoding
 * @note if EC_Level is > 3 it gets automatically adjusted to 3
 * @note the encoding mode gets selected automatically
 */
void QRC::Encode(char const *Message, int EC_Level)
{
    // EC_Level:
    // 0 = recovers  7% of data
    // 1 = recovers 15% of data
    // 2 = recovers 25% of data
    // 3 = recovers 30% of data
    // anything else => 0
    
    // ec level correcting
    switch(EC_Level)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            this->EC = EC_Level;
            break;
        default:
            this->EC = 3;
            break;
    }
    
    // loop through whole message -> determine correct mode
    int Numeric = 0;
    int Alphanumeric = 0;
    int Byte = 0;
    
    for(int i = 0; ; i++)
    {
        this->ch = Message[i];
        if(this->ch == '\0') break;
        if(this->IsNumeric()) Numeric++;
        if(this->IsAlphanumeric()) Alphanumeric++;
        if(this->IsByte()) Byte++;
    }
    
    this->Count = Byte;   // count is equal to the bytes
    
    // debug information
    DEBUG(printf("N: %d\n", Numeric));
    DEBUG(printf("A: %d\n", Alphanumeric));
    DEBUG(printf("A: %d\n", Byte));
    
    // select the mode
    if(Byte > Alphanumeric) this->Mode = MODE_BYTE;                 // Byte mode
    else if(Alphanumeric > Numeric) this->Mode = MODE_ALPHANUMERIC; // Alphanumeric Mode
    else this->Mode = MODE_NUMERIC;                                 // Byte Mode
    
    DEBUG(std::cout << "Message is: " << Message << std::endl);
    
    // determine the version
    this->Version = 0;
    for(int i = 0; i < 40; i++)
    {
        if(qrlimits[i][this->Mode][EC_Level] >= this->Count) 
        {
            this->Version = i + 1;
            break;
        }
    }
    if(!this->Version)  // no version found!
    {
        return;
    }
    else
    {
        this->Version--;
    }
    
    DEBUG(std::cout << "selected version: " << this->Version + 1 << std::endl);
    DEBUG(std::cout << "selected EC-level " << this->EC << std::endl);

    // append mode indicator
    DEBUG(printf("Mode indicator...\n"));
    switch(this->Mode)
    {
        case MODE_NUMERIC:
            DEBUG(printf("...numeric:\n"));
            QRC::AppendNumber(&this->Raw, 1, 4);  // 0001
            break;
        case MODE_ALPHANUMERIC:
            DEBUG(printf("...alphanumeric:\n"));
            QRC::AppendNumber(&this->Raw, 2, 4);  // 0010
            break;
        case MODE_BYTE:
            DEBUG(printf("...byte:\n"));
            QRC::AppendNumber(&this->Raw, 4, 4);  // 0100
            break;
        default:
        // TODO ERROR
            break;
    }
    
    // append the character count indicator
    DEBUG(printf("Character count indicator:\n"));
    if(this->Version >= 0 && this->Version <= 8) 
    {
        QRC::AppendNumber(&this->Raw, this->Count, qrcharcount[VER_1_9][this->Mode]);
    }
    if(this->Version >= 9 && this->Version <= 25) 
    {
        QRC::AppendNumber(&this->Raw, this->Count, qrcharcount[VER_10_26][this->Mode]);
    }
    if(this->Version >= 26 && this->Version <= 39)
    {
        QRC::AppendNumber(&this->Raw, this->Count, qrcharcount[VER_27_40][this->Mode]);
    }
    
    // encode using the correct mode. (the 2nd step 3 of the following link)
    // see: https://www.thonky.com/qr-code-tutorial/data-encoding
    DEBUG(printf("Encode:\n"));
    switch(this->Mode)
    {
        case MODE_NUMERIC:
        {
            // determine new raw data length
            int TargetLength = (int)(strlen(Message) / 3) + 1;
            
            DEBUG(std::cout << "target length " << TargetLength << std::endl);
            
            // allocate new length
//            if(this->Raw.Data) delete this->Raw.Data;
//            this->Raw.Data = new uint32_t[this->Raw.Length];
//@TODO       
            // Numeric mode encoding
            // https://www.thonky.com/qr-code-tutorial/numeric-mode-encoding
            for(int i = 0; i < TargetLength; i++)
            {
                bool Appent = false;
                // get the three-digit number
                unsigned int Number3 = 0;
                if(Message[3 * i]) 
                {
                    Number3 = Message[3 * i] - '0';
                    if(Message[3 * i + 1]) 
                    {
                        Number3 = Number3 * 10 + Message[3 * i + 1] - '0';
                        if(Message[3 * i + 2]) 
                        {
                            Number3 = Number3 * 10 + Message[3 * i + 2] - '0';
                            QRC::AppendNumber(&this->Raw, Number3, 10);
                            Appent = true;
                        }
                        if(!Appent) QRC::AppendNumber(&this->Raw, Number3, 7);
                        Appent = true;
                    }
                    if(!Appent) QRC::AppendNumber(&this->Raw, Number3, 4);
                }
                
                DEBUG(std::cout << "Three/To/One digit number: " << Number3 << std::endl);
            }
            
            break;
        }
        case MODE_ALPHANUMERIC:
        {
            int TotalLength = strlen(Message);
            int TargetLength = (int)(TotalLength / 2);
            int i = 0;
            
            for(i = 0; i < TargetLength; i++)
            {
                // append to raw: combine characters and pad up to 11
                QRC::AppendNumber(&this->Raw, 45 * this->AlphanumericCode(Message[2 * i]) + this->AlphanumericCode(Message[2 * i + 1]), 11);
            }
            
            if(TotalLength % 2)
            {
                // append one character to raw and pad up to 6
                QRC::AppendNumber(&this->Raw, this->AlphanumericCode(Message[2 * i]), 6);
            }
            
            break;
        }
        case MODE_BYTE:
        {
            int TotalLength = strlen(Message);
            for(int i = 0; i < TotalLength; i++)
            {
                QRC::AppendNumber(&this->Raw, Message[i], 8);
            }
            break;
        }
        default:
        {
            break;
        }
    }
    
    // copy nb of things
    this->Nb.B1 = qrECBI[this->Version][this->EC][NB_B1];
    DEBUG(printf("Nb.B1: %d\n", this->Nb.B1));
    this->Nb.B2 = qrECBI[this->Version][this->EC][NB_B2];
    DEBUG(printf("Nb.B2: %d\n", this->Nb.B2));
    this->Nb.Data_CW_B1 = qrECBI[this->Version][this->EC][NB_DATA_CW_B1];
    DEBUG(printf("Nb.Data_CW_B1: %d\n", this->Nb.Data_CW_B1));
    this->Nb.Data_CW_B2 = qrECBI[this->Version][this->EC][NB_DATA_CW_B2];
    DEBUG(printf("Nb.Data_CW_B2: %d\n", this->Nb.Data_CW_B2));
    this->Nb.EC_CW_Per_Block = qrECBI[this->Version][this->EC][NB_EC_CW_PER_BLOCK];
    DEBUG(printf("Nb.EC_CW_Per_Block: %d\n", this->Nb.EC_CW_Per_Block));
    
    // determine version
    
    // break up into 8-bit codewords and pad bytes if necessairy (step 4, the 2nd one)
    // see: https://www.thonky.com/qr-code-tutorial/data-encoding
    
    // pad up to 4
    DEBUG(std::cout << "Next up: Padding of up to 4 zero bits" << std::endl);
    int TotalBytes = this->Nb.B1 * this->Nb.Data_CW_B1 + this->Nb.B2 * this->Nb.Data_CW_B2;
    int PadBytes = TotalBytes - this->Raw.Data.size();
    int PadBits = 8 * PadBytes;
    if(PadBits > 4)
    {
        PadBits = 4;
    }
    QRC::AppendNumber(&this->Raw, 0, PadBits);
    
    // pad until bytes are multiple of 8
    DEBUG(std::cout << "Next up: pad until bytes are multiple of 8" << std::endl);
    PadBits = (8 - (this->Raw.Offset % 8)) & 0x7;   // take last three bits of 8 - modulo 8
    QRC::AppendNumber(&this->Raw, 0, PadBits);
    
    // add pad bytes if still too short (236 & 17)
    DEBUG(std::cout << "Next up: pad with 236 or 7" << std::endl);
    PadBytes = TotalBytes - this->Raw.Data.size();
    for(int i = 0; i < PadBytes; i++)
    {
        if(i % 2)   // is the current iteration uneven?
        {
            QRC::AppendNumber(&this->Raw, 17, 8);
        }
        else    // even
        {
            QRC::AppendNumber(&this->Raw, 236, 8);
        }
    }
    
    // create: "generator polynomial"
    // https://www.thonky.com/qr-code-tutorial/error-correction-coding
    this->CreateGeneratorPoly(&this->GeneratorPoly, this->Nb.EC_CW_Per_Block);
    
    DEBUG(std::cout << "Generator Polynomial ";)
    DEBUG(std::cout << "(" << this->GeneratorPoly.size() << " bytes): ";)
    DEBUG(for(int i = 0; i < (int)this->Nb.EC_CW_Per_Block + 1; i++))
    DEBUG(std::cout << static_cast<int16_t>(this->GeneratorPoly[i]) << ",";)
    DEBUG(std::cout << "\b " << std::endl;)
    
    DEBUG(std::cout << "Message ";)
    DEBUG(std::cout << "(" << this->Raw.Data.size() << " bytes): ";)
    DEBUG(for(int i = 0; i < (int)this->Raw.Data.size(); i++))
    DEBUG(printf("%3d,", this->Raw.Data[i]);)
//    DEBUG(std::cout << static_cast<int16_t>(this->Raw.Data[i]) << ",";)
    DEBUG(std::cout << "\b " << std::endl;)
    DEBUG(std::cout << "Message hex ";)
    DEBUG(std::cout << "(" << this->Raw.Data.size() << " bytes): ";)
    DEBUG(for(int i = 0; i < (int)this->Raw.Data.size(); i++))
    DEBUG(printf("%02X ", this->Raw.Data[i]);)
    DEBUG(std::cout << "\b " << std::endl;)
    
    // create: "error correction codewords"
    this->ECCodewords = this->CreateMessagePoly(this->Raw.Data, this->GeneratorPoly, this->Nb);
    
    // structure final message
    this->Final = this->CreateFinal(this->Raw, this->ECCodewords, this->Nb, this->Version);
    
    // create module with finder patterns etc
    this->Module = QRC::CreateModule(this->Version);
    
    // place final data in module
    this->Module = QRC::PlaceInModule(this->Module, this->Final, this->Version);
    
    // get the most ideal module
    this->Mask = QRC::GetOptimalMask(this->Module, this->EC, this->Version);
    
    // apply mask
    DEBUG(printf("...apply best mask...\n"));
    this->Module = QRC::ApplyMask(this->Module, this->Mask, this->Version);
    
    // add format and version information
    DEBUG(printf("...add format and version information...\n"));
    this->Module = QRC::PlaceFmtVer(this->Module, this->EC, this->Mask, this->Version);
    
    DEBUG(printf("...final QR code:\n"));
    DEBUG(display(this->Module, this->Version));
    display(this->Module, this->Version);
    
}

/**
 * @brief export an encoded QR-code as a windows bitmap file.
 * @note does not work
 * @param Name - output filename 
 */
void QRC::ExportAsBMP(std::string Name)                // export it as a bitmap
{
    int Dr = this->Version * 4 + 21;      // dimensions (raw, one "pixel")
    int Dw = ((Dr - 1) >> 3) + 1;   // dimension (width, bytes)
    
    std::ofstream file;
    //file.open(Name, std::ios::out | std::ios::app | std::ios::binary | std::ios::trunc);
    file.open(Name, std::ofstream::binary | std::ofstream::trunc);
    if(!file.is_open()) return;
    
    // info header
    uint32_t biSizeImage = (this->Module.size());    // biSizeImage
    // file header
    int16_t bfType = 0x4d42;    // (BM)
    int32_t bfOffBits = 40 + 14 + 2 * 4;    // 2 * 4 bytes (32 bits each) for 2 entries in clrs (the lookup table for colours)
    uint32_t bfSize = bfOffBits + biSizeImage;
    uint32_t bfRes = 0;
    file.write((char*)&bfType, sizeof bfType); // bfType (BM)
    file.write((char*)&bfSize, sizeof bfSize);    // bfSize
    file.write((char*)&bfRes, sizeof bfRes); // res 1 & 2
    file.write((char*)&bfOffBits, sizeof bfOffBits);
    // info header
    uint32_t biSize = (40); // biSize
    file.write((char*)&biSize, sizeof biSize);
    int32_t biWidth = (Dr + 2);  // biWidth + 2 (left and right border)
    file.write((char*)&biWidth, sizeof biWidth);
    int32_t biHeight = (Dr + 2);  // biHeight + 2 (upper and lower border)
    file.write((char*)&biHeight, sizeof biHeight);
    int16_t biPlanes = (1);   // biPlanes
    file.write((char*)&biPlanes, sizeof biPlanes);
    int16_t biBitCount = (1);   // biBitCount
    file.write((char*)&biBitCount, sizeof biBitCount);
    uint32_t biCompression = (0);  // biCompression
    file.write((char*)&biCompression, sizeof biCompression);
    file.write((char*)&biSizeImage, sizeof biSizeImage);
    uint32_t biXPelsPerMeter = (0);  // biXPelsPerMeter
    file.write((char*)&biXPelsPerMeter, sizeof biXPelsPerMeter);
    uint32_t biYPelsPerMeter = (0);  // biYPelsPerMeter
    file.write((char*)&biYPelsPerMeter, sizeof biYPelsPerMeter);
    uint32_t biClrUsed = (2);  // biClrUsed
    file.write((char*)&biClrUsed, sizeof biClrUsed);
    uint32_t biClrImportant = (0);  // biClrImportant
    file.write((char*)&biClrImportant, sizeof biClrImportant);
    // black and white
    uint32_t b = 0x00FFFFFF;
    uint32_t w = 0x00000000;
    file.write((char*)&b, sizeof b);
    file.write((char*)&w, sizeof w);
    // image data
    int DwSL = ((Dr + 1) >> 3) + 1;
    int scanline = ((3 + DwSL) / 4) * 4 * 8;
    for(int i = Dr + 1; i + 1 > 0; i--)
    {
        char data = 0;
        int bits = 0;
        bool border = false;
        if(!i || i == Dr + 1)
        {
            border = true;
        }
        for(int j = 0; j < scanline; j++)
        {
            if(border || !j || j == Dr + 1)
            {
                data |= (0 << (7 - bits));
            }
            else if(j && j < Dr + 1)
            {
                data |= ((MODULE_get(j - 1, i - 1) & 0b1) << (7 - bits));//data |= (1 << (7 - bits));
            }
            bits++;
            if(bits >= 8)
            {
                bits = 0;
                file.write(&data, 1);
                data = 0;
            }
            
        }
    }
    
    file.close();
}

/**
 * @brief copy the final QR-Code into an array
 * @param Array - the array, which is a 2D-Array represented as a 1D-Array.
 * @param width - width of the array.
 * @param height - height of the array
 * @param x0 - desired x-position
 * @param y0 - desired y-position
 * @param FitToPx - the QR-Code will be stretched to at most or less than FitToPx
 * @note the x and y coordinates are top left. The final pixel is bottom right.
 * @note the array is defined as such: MSB in first byte is the top left pixel. LSB in the last byte is the bottom right
 *       pixel. Direction is from left to right and top to bottom.
 */
void QRC::CopyIntoArray(uint8_t *Array, int width, int height, int x0, int y0, int FitToPx)
{
    // check how far we can stretch
    int Dr = this->Version * 4 + 21;      // dimensions (raw, one "pixel")
    int Dw = ((Dr - 1) >> 3) + 1;   // dimension (width, bytes)
    int stretch = FitToPx / Dr;
    int realxreal = stretch * Dr;    // real pixel by pixel size
    
    // modify coordinates for center adjustment
    int xarray0 = x0 + ((FitToPx - realxreal) >> 1);
    int yarray0 = y0 + ((FitToPx - realxreal) >> 1);
    
    int x = xarray0;
    int y = yarray0;
    
    // plot into array
    for(int ycode = 0; ycode < Dr; ycode++)
    {
        x = xarray0;
        for(int xcode = 0; xcode < Dr; xcode++)
        {
            uint8_t pixel = (~MODULE_get(xcode, ycode)) & 1;
            for(int sy = 0; sy < stretch; sy++)
            {
                for(int sx = 0; sx < stretch; sx++)
                {
                    // plot into array
                    // x-coordinate: x + x0 + sx
                    // y-coordinate: y + y0 + sy
                    int xcoord = (x - xarray0) * stretch + xarray0 + sx;
                    int ycoord = (y - yarray0) * stretch + yarray0 + sy;
                    int bit = ycoord * width + xcoord;
                    int byte = bit >> 3;
                    int shift = 7 - (bit % 8);
                    // check that we're inbounds
                    if(bit > width * height)
                    {
                        // out of bounds
                        return;
                    }
                    // erase
                    Array[byte] &= ~(1 << shift);
                    // place
                    Array[byte] |= (pixel << shift);
                }
            }
            x++;
            if(x > width)
            {
                return;
            }
        }
        y++;
        if(y > height)
        {
            break;
        }
    }
}




#undef MODULE_xy
#undef MODULE_get
#undef MODULE_clear
#undef MODULE_write