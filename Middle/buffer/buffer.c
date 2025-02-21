
#include "buffer.h"
#include "stdlib.h"

#if USING_SLIP == 1
#define SLIP_END                0xC0
#define SLIP_ESC                0xDB
#define SLIP_ESC_END            0xDC
#define SLIP_ESC_ESC            0xDD
#endif

#define CRC8_TABLELENGTH           256u

static const uint8_t CRC8_J1850_TABLE[CRC8_TABLELENGTH] =
{
        0x00 , 0x1D , 0x3A , 0x27 , 0x74 , 0x69 , 0x4E , 0x53 ,
        0xE8 , 0xF5 , 0xD2 , 0xCF , 0x9C , 0x81 , 0xA6 , 0xBB ,
        0xCD , 0xD0 , 0xF7 , 0xEA , 0xB9 , 0xA4 , 0x83 , 0x9E ,
        0x25 , 0x38 , 0x1F , 0x02 , 0x51 , 0x4C , 0x6B , 0x76 ,
        0x87 , 0x9A , 0xBD , 0xA0 , 0xF3 , 0xEE , 0xC9 , 0xD4 ,
        0x6F , 0x72 , 0x55 , 0x48 , 0x1B , 0x06 , 0x21 , 0x3C ,
        0x4A , 0x57 , 0x70 , 0x6D , 0x3E , 0x23 , 0x04 , 0x19 ,
        0xA2 , 0xBF , 0x98 , 0x85 , 0xD6 , 0xCB , 0xEC , 0xF1 ,
        0x13 , 0x0E , 0x29 , 0x34 , 0x67 , 0x7A , 0x5D , 0x40 ,
        0xFB , 0xE6 , 0xC1 , 0xDC , 0x8F , 0x92 , 0xB5 , 0xA8 ,
        0xDE , 0xC3 , 0xE4 , 0xF9 , 0xAA , 0xB7 , 0x90 , 0x8D ,
        0x36 , 0x2B , 0x0C , 0x11 , 0x42 , 0x5F , 0x78 , 0x65 ,
        0x94 , 0x89 , 0xAE , 0xB3 , 0xE0 , 0xFD , 0xDA , 0xC7 ,
        0x7C , 0x61 , 0x46 , 0x5B , 0x08 , 0x15 , 0x32 , 0x2F ,
        0x59 , 0x44 , 0x63 , 0x7E , 0x2D , 0x30 , 0x17 , 0x0A ,
        0xB1 , 0xAC , 0x8B , 0x96 , 0xC5 , 0xD8 , 0xFF , 0xE2 ,
        0x26 , 0x3B , 0x1C , 0x01 , 0x52 , 0x4F , 0x68 , 0x75 ,
        0xCE , 0xD3 , 0xF4 , 0xE9 , 0xBA , 0xA7 , 0x80 , 0x9D ,
        0xEB , 0xF6 , 0xD1 , 0xCC , 0x9F , 0x82 , 0xA5 , 0xB8 ,
        0x03 , 0x1E , 0x39 , 0x24 , 0x77 , 0x6A , 0x4D , 0x50 ,
        0xA1 , 0xBC , 0x9B , 0x86 , 0xD5 , 0xC8 , 0xEF , 0xF2 ,
        0x49 , 0x54 , 0x73 , 0x6E , 0x3D , 0x20 , 0x07 , 0x1A ,
        0x6C , 0x71 , 0x56 , 0x4B , 0x18 , 0x05 , 0x22 , 0x3F ,
        0x84 , 0x99 , 0xBE , 0xA3 , 0xF0 , 0xED , 0xCA , 0xD7 ,
        0x35 , 0x28 , 0x0F , 0x12 , 0x41 , 0x5C , 0x7B , 0x66 ,
        0xDD , 0xC0 , 0xE7 , 0xFA , 0xA9 , 0xB4 , 0x93 , 0x8E ,
        0xF8 , 0xE5 , 0xC2 , 0xDF , 0x8C , 0x91 , 0xB6 , 0xAB ,
        0x10 , 0x0D , 0x2A , 0x37 , 0x64 , 0x79 , 0x5E , 0x43 ,
        0xB2 , 0xAF , 0x88 , 0x95 , 0xC6 , 0xDB , 0xFC , 0xE1 ,
        0x5A , 0x47 , 0x60 , 0x7D , 0x2E , 0x33 , 0x14 , 0x09 ,
        0x7F , 0x62 , 0x45 , 0x58 , 0x0B , 0x16 , 0x31 , 0x2C ,
        0x97 , 0x8A , 0xAD , 0xB0 , 0xE3 , 0xFE , 0xD9 , 0xC4
};


Buffer* newBuffer(void)
{
    Buffer* _newBuff = (Buffer*)malloc(sizeof(Buffer));
    if (_newBuff != BUFF_NULL)
    {
        _newBuff->data = BUFF_NULL;
        _newBuff->length = 0;
        _newBuff->link = BUFF_NULL;
    }
    return _newBuff;
}

uint8_t Buffer_CalCrc8( uint8_t InitialValue,  uint8_t* u8DataPtr, uint16_t length)
{
    uint8_t idx;
    uint8_t crc = InitialValue;
    /* calculate CRC8 conform to SAE J1850 */
    for (idx = 0; idx < length; idx++)
    {
        crc = CRC8_J1850_TABLE[crc ^ (u8DataPtr[idx])];
    } /* the calculate CRC value shall be XOR with TUINT8_MAX ! */
    crc ^= InitialValue;
    return crc;
}


Buff_ReturnType Buffer_AddData(Buffer* buff, uint8_t* u8DataPtr, uint16_t length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;

    Buffer* _buff = buff;
    Buffer* head_buff = buff;

    while (_buff != BUFF_NULL)
    {
        if (_buff->length != 0 && _buff->data != BUFF_NULL)
        {
            head_buff = _buff;
            _buff = _buff->link;
        }
        else
        {
            _buff->data = (uint8_t*)malloc(length);
            if (_buff->data == BUFF_NULL)
            {
                state = BUFF_UNLOCK;
                return BUFF_NOT_OK;
            }

            for (uint16_t countByte = 0; countByte < length; countByte++)
            {
                _buff->data[countByte] = u8DataPtr[countByte];
            }
            _buff->length = length;
            state = BUFF_UNLOCK;
            return BUFF_OK;
        }
    }

    _buff = (Buffer*)malloc(sizeof(Buffer));
    if (_buff == BUFF_NULL)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    _buff->data = (uint8_t*)malloc(length);
    if (_buff->data == BUFF_NULL)
    {
        free(_buff);
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    _buff->length = 0;
    _buff->link = BUFF_NULL;
    if (head_buff != BUFF_NULL)
    {
        head_buff->link = _buff;
    }

    for (uint16_t countByte = 0; countByte < length; countByte++)
    {
        _buff->data[countByte] = u8DataPtr[countByte];
    }
    _buff->length = length;
    state = BUFF_UNLOCK;
    return BUFF_OK;
}

uint8_t* Buffer_GetPtrData(Buffer* buff, uint16_t position)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;

    Buffer* _buff = buff;
    uint16_t Count = Buffer_CountData(buff);

    if (position >= Count)
    {
        state = BUFF_UNLOCK;
        return BUFF_NULL;
    }

    for (uint16_t countData = 0; countData < position; countData++)
    {
        _buff = _buff->link;
    }
    state = BUFF_UNLOCK;
    return _buff->data;
}

Buffer* Buffer_GetPtrLink(Buffer* buff, uint16_t position)
{
    Buffer* _buff = buff;
    uint16_t Count = Buffer_CountData(buff);

    if (position >= Count)
    {
        return BUFF_NULL;
    }

    for (uint16_t countData = 0; countData < position; countData++)
    {
        _buff = _buff->link;
    }
    return _buff;
}

uint16_t Buffer_GetSizeData(Buffer* buff, uint16_t position)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;

    Buffer* _buff = buff;
    uint16_t Count = Buffer_CountData(buff);
    
    if (position >= Count)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    for (uint16_t countData = 0; countData < position; countData++)
    {
        _buff = _buff->link;
    }
    state = BUFF_UNLOCK;
    return _buff->length;
}

Buff_ReturnType Buffer_GetData(Buffer* buff, uint16_t position, uint8_t* u8DataPtr, uint16_t* length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;

    Buffer* _buff = buff;
    uint16_t Count = Buffer_CountData(buff);

    if (position >= Count || _buff->length == 0)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    for (uint16_t countData = 0; countData < position; countData++)
    {
        _buff = _buff->link;
    }

    for (uint16_t countByte = 0; countByte < _buff->length; countByte++)
    {
        u8DataPtr[countByte] = _buff->data[countByte];
    }
    *length = _buff->length;
    state = BUFF_UNLOCK;
    return BUFF_OK;
}

Buff_ReturnType Buffer_AppendData(Buffer* buff, uint16_t position, uint8_t* u8DataPtr, uint16_t length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;

    Buffer* _buff = buff;
    uint16_t Count = Buffer_CountData(buff);
    
    if (position >= Count)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    for (uint16_t countData = 0; countData < position; countData++)
    {
        _buff = _buff->link;
    }

    uint8_t* newdata = (uint8_t*)realloc(_buff->data, _buff->length + length);
    if (newdata == BUFF_NULL)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }
    _buff->data = newdata;

    for (uint16_t countByte = 0; countByte < length; countByte++)
    {
        _buff->data[_buff->length + countByte] = u8DataPtr[countByte];
    }
    _buff->length = _buff->length + length;

    state = BUFF_UNLOCK;
    return BUFF_OK;
}

Buff_ReturnType Buffer_ReplaceData(Buffer* buff, uint16_t position, uint8_t* u8DataPtr, uint16_t length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    Buffer* currbuff = buff;
    Buffer* headbuff = buff;
    Buffer* tailbuff = buff;

    Buffer* newbuff = (Buffer*)malloc(sizeof(Buffer));
    uint16_t Count = Buffer_CountData(buff);

    if (position >= Count || newbuff == BUFF_NULL)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }
    for (uint16_t countData = 0; countData < position; countData++)
    {
        headbuff = currbuff;
        if (currbuff != BUFF_NULL)
        {
            currbuff = currbuff->link;
            tailbuff = currbuff->link;
        }
    }

    newbuff->data = (uint8_t*)malloc(length);
    if (newbuff->data == BUFF_NULL)
    {
        free(newbuff);
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    for (uint16_t countByte = 0; countByte < length; countByte++)
    {
        newbuff->data[countByte] = u8DataPtr[countByte];
    }
    newbuff->length = length;
    newbuff->link = (tailbuff != currbuff) ? tailbuff : BUFF_NULL;

    if (headbuff != currbuff)
    {
        headbuff->link = newbuff;
    }
    else
    {
        buff = newbuff;
    }

    if (currbuff != BUFF_NULL)
    {
        free(currbuff->data);
        free(currbuff);
    }
    state = BUFF_UNLOCK;
    return BUFF_OK;
}

Buff_ReturnType array_equal(const uint8_t* array1, uint16_t size1, const uint8_t* array2, uint16_t size2)
{
    int flag = BUFF_OK;

    if (size1 != size2)
    {
        return flag = BUFF_NOT_OK;
    }

    for (uint16_t i = 0; i < size1; ++i)
    {
        if (array1[i] != array2[i])
        {
            return flag = BUFF_NOT_OK;
        }
    }
    return flag;
}

Buff_ReturnType Buffer_SearchData(Buffer* buff, uint8_t* u8DataPtr, uint16_t length, uint16_t* position)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    Buffer* _buff = buff;
    uint16_t pos = 0;

    while (_buff != BUFF_NULL)
    {
        if (BUFF_OK == array_equal(_buff->data, _buff->length, u8DataPtr, length))
        {
            *position = pos;
            state = BUFF_UNLOCK;
            return BUFF_OK;
        }

        pos++;
        _buff = _buff->link;
    }
    pos = 0xFFFF;
    state = BUFF_UNLOCK;
    return BUFF_NOT_OK;
}


Buff_ReturnType Buffer_SplitData(Buffer* buff, uint16_t posBuff, uint16_t posData)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    Buffer* _buff = buff;
    Buffer* next_buff = _buff->link;

    uint16_t Count = Buffer_CountData(buff);

    if (posBuff >= Count)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    for (uint16_t countData = 0; countData < posBuff; countData++)
    {
        _buff = _buff->link;
        next_buff = _buff->link;
    }

    if (posData == 0 || posData > _buff->length)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    uint8_t* newdatahead = (uint8_t*)malloc(posData - 1);
    uint8_t* newdatatail = (uint8_t*)malloc(_buff->length - posData + 1);
    Buffer* newbuff = newBuffer();

    if (newdatahead == BUFF_NULL || newdatatail == BUFF_NULL || newbuff == BUFF_NULL)
    {
        free(newdatahead);
        free(newdatatail);
        free(newbuff);
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    for (uint16_t countData = 0; countData < _buff->length; countData++)
    {
        if (countData < (posData - 1))
        {
            newdatahead[countData] = _buff->data[countData];
        }
        else
        {
            newdatatail[countData - posData + 1] = _buff->data[countData];
        }
    }

    newbuff->data = newdatatail;
    newbuff->length = _buff->length - posData + 1;
    newbuff->link = next_buff;

    free(_buff->data);
    _buff->data = newdatahead;
    _buff->length = posData - 1;
    _buff->link = newbuff;

    state = BUFF_UNLOCK;
    return BUFF_OK;
}

Buff_ReturnType Buffer_MergeData(Buffer* buff, uint16_t posSrc, uint16_t posDes)
{
    return BUFF_OK;
}

Buff_ReturnType Buffer_MergeAllData(Buffer* buff)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    Buffer* _buff = buff->link;

    while (_buff != BUFF_NULL)
    {
        uint8_t *newdata = (uint8_t*)realloc(buff->data, buff->length + _buff->length);
        if (newdata == BUFF_NULL)
        {
            state = BUFF_UNLOCK;
            return BUFF_NOT_OK;
        }
        buff->data = newdata;
        for (uint16_t countByte = 0; countByte < _buff->length; countByte++)
        {
            newdata[buff->length + countByte] = _buff->data[countByte];
        }
        buff->length += _buff->length;
        _buff = _buff->link;
    }
    
    Buffer_Dispose(buff->link);
    buff->link = BUFF_NULL;
    state = BUFF_UNLOCK;
    return BUFF_OK;
}

Buff_ReturnType Buffer_DeleteData(Buffer* buff, uint16_t position)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    Buffer* lastbuff = buff;
    Buffer* currbuff = buff;

    uint16_t Count = Buffer_CountData(buff);

    if (position >= Count)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    for (uint16_t countData = 0; countData < (Count - 1); countData++)
    {
        if (countData >= position)
        {
            if (currbuff->data != BUFF_NULL && countData == position)
            {
                free(currbuff->data);
                currbuff->data = NULL;
            }
            currbuff->data = currbuff->link->data;
            currbuff->length = currbuff->link->length;
        }
        lastbuff = currbuff;
        currbuff = currbuff->link;       
    }

    lastbuff->link = BUFF_NULL;
    if (Count == 1)
    {
        if (currbuff->data != BUFF_NULL)
        {
            free(currbuff->data);
        }
        currbuff->data = BUFF_NULL;
        currbuff->length = 0;
        currbuff->link = BUFF_NULL;

    }
    else
    {
        free(currbuff);
        currbuff = NULL;
    }

    state = BUFF_UNLOCK;
    return BUFF_OK;
}

uint16_t Buffer_CountData(Buffer* buff)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;

    Buffer* _buff = buff;
    uint16_t countData = 0;

    while (_buff != BUFF_NULL )
    {
        countData++;
        if(_buff->length == 0)
        {
        	countData --;
        }
        _buff = _buff->link;
        if(_buff == buff)
        {
        	//_buff = BUFF_NULL;
        }
    }

    state = BUFF_UNLOCK;
    return countData;
}

void Buffer_Dispose(Buffer* buff)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    uint16_t nbData = Buffer_CountData(buff);
    for (uint16_t countData = 0; countData < nbData; countData++)
    {
        Buffer_DeleteData(buff, 0);
    }
    free(buff);
    buff = NULL;
    state = BUFF_UNLOCK;
}

#if USING_SLIP == 1
Buffer* Buffer_SlipSplitPack(uint8_t TypeFrame, uint8_t* u8DataPtr, uint16_t length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    uint8_t _data[2];
    Buffer* buff = newBuffer();
    if (TypeFrame == SLIP_FULLFRAME || TypeFrame == SLIP_HEADFRAME)
    {
        _data[0] = SLIP_END;
        if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, _data, 1))
        {
            Buffer_Dispose(buff);
            state = BUFF_UNLOCK;
            return BUFF_NULL;
        }
    }

    for (uint16_t countByte = 0; countByte < length; ++countByte)
    {
        if (u8DataPtr[countByte] == SLIP_END)
        {
            _data[0] = SLIP_ESC;
            _data[1] = SLIP_ESC_END;
            if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, _data, 2))
            {
                Buffer_Dispose(buff);
                state = BUFF_UNLOCK;
                return BUFF_NULL;
            }
        }
        else if (u8DataPtr[countByte] == SLIP_ESC)
        {
            _data[0] = SLIP_ESC;
            _data[1] = SLIP_ESC_ESC;
            if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, _data, 2))
            {
                state = BUFF_UNLOCK;
                Buffer_Dispose(buff);
                return BUFF_NULL;
            }
        }
        else
        {
            _data[0] = u8DataPtr[countByte];
            if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, _data, 1))
            {
                Buffer_Dispose(buff);
                state = BUFF_UNLOCK;
                return BUFF_NULL;
            }
        }
    }

    if (TypeFrame == SLIP_FULLFRAME || TypeFrame == SLIP_ENDFRAME)
    {
        _data[0] = SLIP_END;
        if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, _data, 1))
        {
            Buffer_Dispose(buff);
            state = BUFF_UNLOCK;
            return BUFF_NULL;
        }
    }
    state = BUFF_UNLOCK;
    return buff;
}

Buffer* Buffer_SlipEnc(uint8_t* u8DataPtr, uint16_t length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    uint8_t _data[2];
    Buffer* buff = newBuffer();

    _data[0] = SLIP_END;
    if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, _data, 1))
    {
        Buffer_Dispose(buff);
        state = BUFF_UNLOCK;
        return BUFF_NULL;
    }

    for (uint16_t countByte = 0; countByte < length; ++countByte)
    {
        if (u8DataPtr[countByte] == SLIP_END)
        {
            _data[0] = SLIP_ESC;
            _data[1] = SLIP_ESC_END;
            if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, _data, 2))
            {
                Buffer_Dispose(buff);
                state = BUFF_UNLOCK;
                return BUFF_NULL;
            }
        }
        else if (u8DataPtr[countByte] == SLIP_ESC)
        {
            _data[0] = SLIP_ESC;
            _data[1] = SLIP_ESC_ESC;
            if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, _data, 2))
            {
                Buffer_Dispose(buff);
                state = BUFF_UNLOCK;
                return BUFF_NULL;
            }
        }
        else
        {
            _data[0] = u8DataPtr[countByte];
            if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, _data, 1))
            {
                Buffer_Dispose(buff);
                state = BUFF_UNLOCK;
                return BUFF_NULL;
            }
        }
    }

    _data[0] = SLIP_END;
    if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, _data, 1))
    {
        Buffer_Dispose(buff);
        state = BUFF_UNLOCK;
        return BUFF_NULL;
    }
    state = BUFF_UNLOCK;
    return buff;
}

Buffer* Buffer_SlipDec(uint8_t* u8DataPtr, uint16_t length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    Buffer* buff = newBuffer();
    uint8_t u8Data_Last = 0;
    uint8_t value = 0;

    if(length == 1)
    { 
    	Buffer_Dispose(buff);
        state = BUFF_UNLOCK;
        return BUFF_NULL;
    }

    for (uint16_t countByte = 0; countByte < length; ++countByte)
    {
        if (u8DataPtr[countByte] == SLIP_ESC)
        {
            u8Data_Last = SLIP_ESC;
        }
        else if (u8DataPtr[countByte] == SLIP_END)
        {
            if (countByte == (length - 1))
            {
                state = BUFF_UNLOCK;
                return buff;
            }
            else
            {
                Buffer_DeleteData(buff, 0);
            }
        }
        else
        {
            if (u8Data_Last == SLIP_ESC)
            {
                u8Data_Last = 0U;
                if (u8DataPtr[countByte] == SLIP_ESC_END)
                {
                    value = SLIP_END;
                    if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, &value, 1))
                    {
                        Buffer_Dispose(buff);
                        state = BUFF_UNLOCK;
                        return BUFF_NULL;
                    }
                }
                else if (u8DataPtr[countByte] == SLIP_ESC_ESC)
                {
                    value = SLIP_ESC;
                    if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, &value, 1))
                    {
                        Buffer_Dispose(buff);
                        state = BUFF_UNLOCK;
                        return BUFF_NULL;
                    }
                }
                else
                {
                    Buffer_Dispose(buff);
                    state = BUFF_UNLOCK;
                    return BUFF_NULL;
                }
            }
            else
            {
                if (BUFF_NOT_OK == Buffer_AppendData(buff, 0, &u8DataPtr[countByte], 1))
                {
                    Buffer_Dispose(buff);
                    state = BUFF_UNLOCK;
                    return BUFF_NULL;
                }
            }
        }
    }

    Buffer_Dispose(buff);
    state = BUFF_UNLOCK;
        return BUFF_NULL;
}
#endif

#if USING_RING == 1
uint16_t Ring_CountByte(Ring *_ring)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    uint16_t count = 0;
    Ring* ring = _ring;
    if (ring->tail >= ring->head)
    {
        count = ring->tail - ring->head;
    }
    else
    {
        count = ring->size - ring->head + ring->tail;
    }
    state = BUFF_UNLOCK;
    return count;
}

uint16_t Ring_CountBytePos(Ring *_ring, uint16_t Pos)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    uint16_t count = 0;
    uint16_t rt_pos = Pos + 1;
    Ring* ring = _ring;

    if (rt_pos == ring->size)
    {
        rt_pos = 0;
    }
    else if (rt_pos > ring->size)
    {
        state = BUFF_UNLOCK;
        return 0;
    }

    if (rt_pos >= ring->head)
    {
        count = rt_pos - ring->head;
    }
    else
    {
        count = ring->size - ring->head + rt_pos;
    }
    state = BUFF_UNLOCK;
    return count;
}

Buff_ReturnType Ring_DeleteData(Ring* _ring, uint16_t Pos)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    if (_ring->tail > _ring->head)
    {
        if (Pos >= _ring->head && Pos < _ring->tail)
        {
            _ring->head = ((Pos + 1) >= _ring->size) ? 0 : (Pos + 1);
            state = BUFF_UNLOCK;
            return BUFF_OK;
        }
        else
        {
            state = BUFF_UNLOCK;
            return BUFF_NOT_OK;
        }
    }
    else
    {
        if (Pos >= _ring->head || Pos < _ring->tail)
        {
            _ring->head = ((Pos + 1) >= _ring->size) ? 0 : (Pos + 1);
            state = BUFF_UNLOCK;
            return BUFF_OK;
        }
        else
        {
            state = BUFF_UNLOCK;
            return BUFF_NOT_OK;
        }
    }


}

Buff_ReturnType Ring_PushData(Ring* _ring, uint8_t* data, uint16_t length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;

    for (uint16_t CountByte = 0; CountByte < length; CountByte++)
    {
        _ring->data[_ring->tail] = data[CountByte];
        if (++_ring->tail == _ring->size)
        {
            _ring->tail = 0;
        }

        if (_ring->tail == _ring->head)
        {
            if (--_ring->tail < 0)
            {
                _ring->tail = _ring->size - 1;
            }
            _ring->data[_ring->tail - 1] = data[CountByte];
        }
    }
    state = BUFF_UNLOCK;
    return BUFF_OK;
}

int16_t Ring_PullData(Ring* _ring, uint8_t* data, uint16_t length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    uint16_t countData = Ring_CountByte(_ring);
    uint16_t maxByteRead = (countData >= length) ? length : countData;

    for (uint16_t CountByte = 0; CountByte < maxByteRead; CountByte++)
    {
        data[CountByte] = _ring->data[_ring->head];
        if (++_ring->head == _ring->size)
        {
            _ring->head = 0;
        }
    }
    state = BUFF_UNLOCK;
    return countData - length;
}

uint16_t Ring_FindByte(Ring *_ring, uint8_t Byte, uint16_t* pos)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    uint16_t PositionByte = _ring->head;
    if (_ring->head == _ring->tail)
    {
        state = BUFF_UNLOCK;
        return 0;
    }
    uint16_t countdata = Ring_CountByte(_ring);

    for (uint16_t CountByte = 0; CountByte < countdata; CountByte++)
    {
        if (_ring->data[PositionByte] == Byte)
        {
            *pos = PositionByte;
            state = BUFF_UNLOCK;
        return CountByte + 1;
        }

        if (++PositionByte == _ring->size)
        {
            PositionByte = 0;
        }
    }
    *pos = 0;
    state = BUFF_UNLOCK;
    return 0;
}

Buff_ReturnType Ring_GetData(Ring* _ring, uint8_t* data, uint16_t* length, uint16_t pos)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    uint16_t newhead = _ring->head;
    for (uint16_t CountByte = 0; CountByte < _ring->size; CountByte++)
    {
        data[CountByte] = _ring->data[newhead];
        if (newhead == pos)
        {
            *length = CountByte + 1;
            _ring->head = (newhead + 1) >= _ring->size ? 0 : (newhead + 1);

            if (pos == _ring->tail)
            {
                *length = CountByte;
                _ring->head = _ring->tail;
            }
            state = BUFF_UNLOCK;
            return BUFF_OK;
        }
        if (newhead == _ring->tail)
        {
            break;
        }

        if (++newhead >= _ring->size)
        {
            newhead = 0;
        }
    }
    state = BUFF_UNLOCK;
    return BUFF_NOT_OK;
}

Ring *newRing(uint16_t size)
{
    uint8_t* dataPtr = (uint8_t*)malloc(size);
    Ring* res = (Ring*)malloc(sizeof(Ring));

    res->size = size;
    res->head = 0;
    res->tail = 0;
    res->data = dataPtr;

    return res;
}

void Ring_Dispose(Ring* ring)
{
    if (ring->data != BUFF_NULL)
    {
        free(ring->data);
        ring->data = BUFF_NULL;
    }

    free(ring);
}
#endif

#if USING_FIFO == 1
Buff_ReturnType Fifo_AddData(Fifo* fifo, void* data, uint16_t length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    
    ListObj* headobj = fifo->objs;
    ListObj* objs = fifo->objs;

    Object  object;
    object.addr = data;
    object.length = length;

    if (fifo->size <= Fifo_CountData(fifo) || object.addr == BUFF_NULL)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    while (objs != BUFF_NULL)
    {
        headobj = objs;
        objs = objs->next;
    }

    ListObj* newobj = (ListObj*)malloc(sizeof(ListObj));
    Object* obj = (Object*)malloc(sizeof(Object));
    void* addr = malloc(object.length);

    if (newobj == BUFF_NULL || obj == BUFF_NULL || addr == BUFF_NULL)
    {
        free(newobj);
        free(obj);
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    obj->addr = addr;
    obj->length = object.length;
    newobj->obj = obj;
    newobj->next = BUFF_NULL;

    uint8_t* datades = (uint8_t*)newobj->obj->addr;
    uint8_t* datasrc = (uint8_t*)object.addr;

    for (uint16_t countByte = 0; countByte < object.length; countByte++)
    {
        datades[countByte] = datasrc[countByte];
    }

    if (headobj == BUFF_NULL)
    {
        fifo->objs = newobj;
    }
    else
    {
        headobj->next = newobj;
    }

    state = BUFF_UNLOCK;
    return BUFF_OK;
}

uint16_t Fifo_GetSizeData(Fifo* fifo)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    ListObj* objs = fifo->objs;
    if (objs == BUFF_NULL)
    {
        state = BUFF_UNLOCK;
        return 0;
    }
    state = BUFF_UNLOCK;
    return objs->obj->length;
}

Buff_ReturnType Fifo_GetData(Fifo* fifo, void* data, uint16_t* length)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    if (data == BUFF_NULL || fifo == BUFF_NULL)
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

    ListObj* objs = fifo->objs;

    if (objs != BUFF_NULL)
    {
        uint8_t* datades = (uint8_t*)data;
        uint8_t* datasrc = (uint8_t*)objs->obj->addr;

        for (uint16_t countByte = 0; countByte < objs->obj->length; countByte++)
        {
            datades[countByte] = datasrc[countByte];
        }
        *length = objs->obj->length;
        fifo->objs = fifo->objs->next;

        free(objs->obj->addr);
        free(objs->obj);
        free(objs);
    }
    else
    {
        state = BUFF_UNLOCK;
        return BUFF_NOT_OK;
    }

        state = BUFF_UNLOCK;
        return BUFF_OK;
}

uint16_t Fifo_CountData(Fifo *fifo)
{
    static uint8_t state = BUFF_UNLOCK;
    while(state == BUFF_LOCK){}
    state = BUFF_LOCK;
    uint16_t countNode = 0;
    ListObj* objs = fifo->objs;

    while (objs != BUFF_NULL)
    {
        objs = objs->next;
        countNode ++;
        if(countNode >= fifo->size)
        {
        	countNode = 0;
        	break;
        }
    }

    state = BUFF_UNLOCK;
    return countNode;
}

void Fifo_Dispose(Fifo* fifo)
{
    uint8_t* data;
    uint16_t NbNode = Fifo_CountData(fifo);
    for (uint16_t CountNode = 0; CountNode < NbNode; CountNode  ++)
    {
        uint16_t size = Fifo_GetSizeData(fifo);
        data = (uint8_t *)malloc(size);
        if (data != BUFF_NULL)
        {
            Fifo_GetData(fifo, data, &size);
            free(data);
            data = BUFF_NULL;
        }
    }

    if (fifo != BUFF_NULL)
    {
        free(fifo);
    }
}

Fifo* newFifo(uint16_t size)
{
    Fifo *res = (Fifo *)malloc(sizeof(Fifo));
    res->size = size;
    res->objs = BUFF_NULL;
    return res;
}
#endif
