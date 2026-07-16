/*++
* Module Name:
*
*     crypto.c
*
* Abstract:
*
*     This module contains crypto routines.
*
* Author:
*
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../headers/crypto.h"

#define DIFF_FLOW_ROTATE_CONSTANT 0x6A09E667UI32

HR_STATUS
FASTCALL
CryDiffusionDataFlow(
    IN OUT UINT8 *pData,
    IN UINT64 DataSize
)
/*++
* Routine Description:
*
*     This routine performs flow diffusion of data
*     based on a sequential mathematical dependency among the
*     states of bit fields.
*
* Arguments:
*
*     pData    - Supplies a pointer to the data.
*
*     DataSize - Supplies the size of the data.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    UINT8 SrcNib = 0;
    UINT8 KeyNib = 0;

    if (!pData || !DataSize) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    for (UINT64 i = 0; ; i++) {
        SrcNib = (pData[i] >> 4);
        KeyNib = (pData[i] & 0x0F);
        KeyNib = (KeyNib ^ (UINT8)_rotr(
            DIFF_FLOW_ROTATE_CONSTANT,
            (int)(KeyNib * KeyNib))) & 0x0F;
        pData[i] &= 0x0F;
        pData[i] |= ((SrcNib ^ KeyNib) << 4);

        //
        // This allows the 4-bit "anchor" to be preserved for de-diffusion.
        //
        if (i == (DataSize - 1)) {
            break;
        }

        SrcNib = (pData[i] & 0x0F);
        KeyNib = (pData[i + 1] >> 4);
        KeyNib = (KeyNib ^ (UINT8)_rotl(
            DIFF_FLOW_ROTATE_CONSTANT,
            (int)(KeyNib * KeyNib))) & 0x0F;
        pData[i] &= 0xF0;
        pData[i] |= (SrcNib ^ KeyNib);
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
CryDeDiffusionDataFlow(
    IN OUT UINT8 *pData,
    IN UINT64 DataSize
)
/*++
* Routine Description:
*
*     This procedure performs chained data de-diffusion
*	  based on a sequential mathematical dependency between the
*     states of bit fields.
*
* Arguments:
*
*     pData    - Supplies a pointer to the data.
*
*     DataSize - Supplies the size of the data.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    UINT8 SrcNib = 0;
    UINT8 KeyNib = 0;

    if (!pData || !DataSize) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    for (UINT64 i = (DataSize - 1); ; i--) {
        SrcNib = (pData[i] >> 4);
        KeyNib = (pData[i] & 0x0F);
        KeyNib = (KeyNib ^ (UINT8)_rotr(
            DIFF_FLOW_ROTATE_CONSTANT,
            (int)(KeyNib * KeyNib))) & 0x0F;
        pData[i] &= 0x0F;
        pData[i] |= ((SrcNib ^ KeyNib) << 4);

        if (!i) {
            break;
        }

        SrcNib = (pData[i - 1] & 0x0F);
        KeyNib = (pData[i] >> 4);
        KeyNib = (KeyNib ^ (UINT8)_rotl(
            DIFF_FLOW_ROTATE_CONSTANT,
            (int)(KeyNib * KeyNib))) & 0x0F;
        pData[i - 1] &= 0xF0;
        pData[i - 1] |= (SrcNib ^ KeyNib);
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
CryFillBufferRandomDword(
    OUT UINT32 *pBuffer,
    IN  UINT32 NoDword,
    IN  UINT32 Seed
)
/*++
* Routine Description:
*
*     This routine fills the specified buffer with random DWORDs.
*
* Arguments:
*
*     pBuffer - Supplies a pointer to the buffer.
*
*     NoDword - Supplies the number of DWORDs.
*
*     Seed    - Supplies the random seed.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    UINT64 Tsc = 0;

    if (!pBuffer || !NoDword) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    Seed = (Seed & 0x1FF) + 0x401;

    Tsc = __rdtsc();
    Tsc = (Tsc >> 32) ^ Tsc;
    Tsc = (Tsc << 32) ^ Tsc;
    Tsc = _byteswap_uint64(Tsc);

    for (UINT32 i = 0; i < NoDword; i++) {
        pBuffer[i] = (UINT32)((Tsc % (i + 1)) & 0x01
            ? _rotr64(Tsc, Seed--)
            : _rotl64(Tsc, Seed++));
    }
    if (HR_ERROR(CryDiffusionDataFlow(
        (UINT8*)pBuffer,
        (UINT64)(NoDword * 4)))) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    return HR_SUCCESS;
}

#define CRC32_POLY 0xEDB88320UI32

HR_STATUS
FASTCALL
CryCrc32DataHash(
    IN  CONST UINT8 *pData,
    IN  UINT64 DataSize,
    OUT UINT32 *pHash
)
/*++
* Routine Description:
*
*     This routine implements the CRC32 hashing algorithm.
*
* Arguments:
*
*     pData    - Supplies a pointer to the data.
*
*     DataSize - Supplies the size of the data.
*
*     pHash    - Supplies a pointer to memory that receives the hash.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    static UINT32 s_Crc32Table[256] = { 0 };
    static BOOLEAN s_TableIsInit = FALSE;

    UINT32 CrcAcc = 0;

    UINT32 Checksum = 0xFFFFFFFF;

    if (!pData || !DataSize) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (!s_TableIsInit) {
        for (UINT32 i = 0; i < 256; i++) {
            CrcAcc = i;
            for (UINT8 i2 = 0; i2 < 8; i2++) {
                if (CrcAcc & 0x01) {
                    CrcAcc = (CrcAcc >> 1) ^ CRC32_POLY;
                }
                else {
                    CrcAcc >>= 1;
                }
            }
            s_Crc32Table[i] = CrcAcc;
        }
        s_TableIsInit = TRUE;
    }

    for (UINT64 i = 0; i < DataSize; i++) {
        Checksum = (Checksum >> 8) ^
            (s_Crc32Table[(pData[i] ^ Checksum) & 0xFF]);
    }

    *pHash = ~Checksum;

    return HR_SUCCESS;
}

