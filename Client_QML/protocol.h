#ifndef PROTOCOL_H
#define PROTOCOL_H

namespace Protocol
{
// Command IDs: first byte of every message the CLIENT sends (TCP 4001).
static const unsigned char CMD_ADD = 0x01;
static const unsigned char CMD_UPDATE = 0x02;
static const unsigned char CMD_DELETE = 0x03;

// Response IDs: first byte of every message the SERVER sends (UDP 4002).
static const unsigned char RESP_ADD = 0x81;
static const unsigned char RESP_UPDATE = 0x82;
static const unsigned char RESP_DELETE = 0x83;

// Ack values from the assignment: 0x00 = fail, 0xFF = success.
static const unsigned char ACK_FAIL = 0x00;
static const unsigned char ACK_SUCCESS = 0xFF;

// pack(1) = no padding between fields. Without it the compiler may insert
// empty bytes, and the struct size would not match what the other side sends.
#pragma pack(push, 1)

// Client -> server. 1 + 4 + 4 + 4 + 50 = 63 bytes.
struct AddDataCmd
{
    unsigned char cmdID;
    unsigned int UniqueID;
    float lat;
    float longi;
    char comment[50];
};

// Server -> client. Same as the command, plus Ack. 64 bytes.
struct AddDataResp
{
    unsigned char respID;
    unsigned char Ack;
    unsigned int UniqueID;
    float lat;
    float longi;
    char comment[50];
};

// Update uses the same layout as Add, only the ID byte differs.
typedef AddDataCmd UpdateDataCmd;
typedef AddDataResp UpdateDataResp;

// Client -> server: 1 + 4 = 5 bytes.
struct DeleteDataCmd
{
    unsigned char cmdID;
    unsigned int UniqueID;
};

// Server -> client: 1 + 1 + 4 = 6 bytes.
struct DeleteDataResp
{
    unsigned char respID;
    unsigned char Ack;
    unsigned int UniqueID;
};

#pragma pack(pop)

// Compile-time check: if a size is wrong, the build fails instead of
// the program misbehaving at runtime.
static_assert(sizeof(AddDataCmd) == 63);
static_assert(sizeof(AddDataResp) == 64);
static_assert(sizeof(DeleteDataCmd) == 5);
static_assert(sizeof(DeleteDataResp) == 6);
}

#endif