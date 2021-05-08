
#ifndef QRCODE_HPP

#define SUPPRESS_DEBUG
#ifndef SUPPRESS_DEBUG
#define DEBUG(code) code
#else
#define DEBUG(code)         // this gets ignored by the compiler (I hope)
#endif

/**********************
 * @category includes *
 **********************/

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

/*******************
 * @category enums *
 *******************/

typedef enum
{
    MODE_NUMERIC,       // has to be 0
    MODE_ALPHANUMERIC,  // has to be 1
    MODE_BYTE,          // has to be 2
}
qrmode_t;

typedef enum 
{
    VER_1_9,
    VER_10_26,
    VER_27_40,
}
qrcount_t;

typedef enum
{
    NB_EC_CW_PER_BLOCK,
    NB_B1,
    NB_DATA_CW_B1,
    NB_B2,
    NB_DATA_CW_B2,
}
qrecbi_t;

typedef struct
{
    int EC_CW_Per_Block;
    int B1;
    int Data_CW_B1;
    int B2;
    int Data_CW_B2;
}
qr_Nb_t;    // number of things

typedef struct
{
    std::vector<uint8_t> Data;      // raw data
    uint32_t Offset;                // raw offset in bits
    //uint32_t Length;    // raw length [of data]
}
raw_t;                    // raw structure

/*******************
 * @category class *
 *******************/

class QRC
{
private:
    /*******************************
     * @category private variables *
     *******************************/
    
    char ch;                // private variable ch for characters
    qrmode_t Mode;          // the current mode that is selected
    int EC;                 // the used error correction level
    int Count;              // the character count
    int Version;            // the current version that is used
    int Mask;               // the current mask used
    std::vector<uint8_t> GeneratorPoly; // generator polynomials
    std::vector<std::vector<std::vector<uint8_t>>> ECCodewords;   // error correction codewords
    std::vector<uint8_t> Module;
    raw_t Raw;              // raw data
    raw_t Final;            // final message
    qr_Nb_t Nb;             // number of things
    
    /*******************************
     * @category private functions *
     *******************************/
    
    bool IsNumeric();       // is private variable ch numeric?
    bool IsAlphanumeric();  // is private variable ch alphanumeric?
    uint8_t AlphanumericCode(char ch);  // get alphanumeric value from a char, see: https://www.thonky.com/qr-code-tutorial/alphanumeric-table
    bool IsByte();          // is private variable ch byte?
    void WriteError(char const *msg, char const *file, int line); // write string to private variable Error
    void AppendNumber(raw_t *Raw, int Number, int Pad);
    void EC_Coding(std::vector<uint8_t> *Data, qr_Nb_t Nb);
    void CreateGeneratorPoly(std::vector<uint8_t> *Polynomials, int Count);
    void CreateECCGB(std::vector<std::vector<std::vector<uint8_t>>> *ECCGB, std::vector<uint8_t> Message, std::vector<uint8_t> Polynomials, int *Mseek, int NbD, int NbB, int NbCW);    // create ecc blocks & groups
    void CreateECCG(std::vector<std::vector<uint8_t>> *ECCG, std::vector<uint8_t> Polynomials, int NbB, int NbCW);
    void CreateECC(std::vector<uint8_t> *ECC, std::vector<uint8_t> Polynomials, int NbCW);
    std::vector<std::vector<std::vector<uint8_t>>> CreateMessagePoly(std::vector<uint8_t> Message, std::vector<uint8_t> Polynomials, qr_Nb_t Nb);
    raw_t CreateFinal(raw_t Raw, std::vector<std::vector<std::vector<uint8_t>>> ECCCW, qr_Nb_t Nb, int Version);
    std::vector<uint8_t> CreateModule(int Version);
    std::vector<uint8_t> PlaceInModule(std::vector<uint8_t> Module, raw_t Final, int Version);
    std::vector<uint8_t> PlaceFmtVer(std::vector<uint8_t> Module, int ECLevel, int Mask, int Version);
    std::vector<uint8_t> ApplyMask(std::vector<uint8_t> Module, int Mask, int Version);
    int GetOptimalMask(std::vector<uint8_t> Module, int ECLevel, int Version);
    bool isFinderPattern(uint16_t Bits, int Length, int LR);
    bool isPlacementLegal(int Version, int x, int y);
public:
    /******************************
     * @category public functions *
     ******************************/
    QRC();
    ~QRC();
    void Encode(char const *Message, int EC_Level); // encode a message to a string
    void ExportAsBMP(std::string Name);               // export it as a bitmap
    void CopyIntoArray(uint8_t *Array, int width, int height, int x0, int y0, int FitToPx);  // copy into array
};


#define QRCODE_HPP
#endif