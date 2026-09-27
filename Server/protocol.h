#ifndef PROTOCOL_H
#define PROTOCOL_H

namespace Protocol
{
// Command IDs
static const unsigned char CMD_ADD = 0x01;
static const unsigned char CMD_UPDATE = 0x02;
static const unsigned char CMD_DELETE = 0x03;

// Response IDs
static const unsigned char RESP_ADD = 0x81;
static const unsigned char RESP_UPDATE = 0x82;
static const unsigned char RESP_DELETE = 0x83;

// ACK
static const unsigned char ACK_FAIL = 0x00;
static const unsigned char ACK_SUCCESS = 0xFF;

#pragma pack(push, 1)

struct AddDataCmd
{
    unsigned char cmdID;
    unsigned int UniqueID;
    float lat;
    float longi;
    char comment[50];
};

struct AddDataResp
{
    unsigned char respID;
    unsigned char Ack;
    unsigned int UniqueID;
    float lat;
    float longi;
    char comment[50];
};

// Update has the same data layout as Add.
typedef AddDataCmd UpdateDataCmd;
typedef AddDataResp UpdateDataResp;

struct DeleteDataCmd
{
    unsigned char cmdID;
    unsigned int UniqueID;
};

struct DeleteDataResp
{
    unsigned char respID;
    unsigned char Ack;
    unsigned int UniqueID;
};

#pragma pack(pop)

static_assert(sizeof(AddDataCmd) == 63);
static_assert(sizeof(AddDataResp) == 64);
static_assert(sizeof(DeleteDataCmd) == 5);
static_assert(sizeof(DeleteDataResp) == 6);
}

#endif