// Protocol.h
// This file just describes the exact format of the data we send between
// the client and the server, so both sides agree on it byte for byte.
#pragma once

#include <QByteArray>
#include <QString>
#include <cstring>

// ---- Port numbers ----
const unsigned short TCP_PORT = 4001;   // client sends commands here
const unsigned short UDP_PORT = 4002;   // server broadcasts responses here
const int COMMENT_LENGTH = 50;

// ---- Command IDs (sent by the client, port 4001) ----
const unsigned char CMD_ADD    = 1;
const unsigned char CMD_UPDATE = 2;
const unsigned char CMD_DELETE = 3;

// ---- Response IDs (sent by the server, port 4002) ----
const unsigned char RESP_ADD    = 1;
const unsigned char RESP_UPDATE = 2;
const unsigned char RESP_DELETE = 3;

// ---- Ack values ----
const unsigned char ACK_FAIL    = 0x00;
const unsigned char ACK_SUCCESS = 0xFF;

// The structures below must match exactly, byte for byte, on both sides.
// "#pragma pack(1)" tells the compiler not to add any hidden padding bytes
// between the fields (it normally adds some to make memory access faster).
// Without this, the two programs could disagree about where one field ends
// and the next begins.
#pragma pack(push, 1)

struct AddDataCmd {            // sent on port 4001, used for both Add and Update
    unsigned char cmdID;
    unsigned int  uniqueID;
    float         lat;
    float         lon;         // sheet says "float long" but "long" is a C++ keyword
    char          comment[COMMENT_LENGTH];
};

struct DeleteDataCmd {         // sent on port 4001
    unsigned char cmdID;
    unsigned int  uniqueID;
};

struct AddDataResp {           // sent on port 4002, used for both Add and Update
    unsigned char respID;
    unsigned char ack;         // 0x00 = fail, 0xFF = success
    unsigned int  uniqueID;
    float         lat;
    float         lon;
    char          comment[COMMENT_LENGTH];
};

struct DeleteDataResp {        // sent on port 4002
    unsigned char respID;
    unsigned char ack;
    unsigned int  uniqueID;
};

#pragma pack(pop)

// These lines are checked while compiling (not while running). They confirm
// the structures above really are the size we expect. If someone changes a
// field and breaks the size, the project simply won't build, instead of
// causing a hidden bug later.
static_assert(sizeof(AddDataCmd)     == 63, "AddDataCmd must be 63 bytes");
static_assert(sizeof(DeleteDataCmd)  == 5,  "DeleteDataCmd must be 5 bytes");
static_assert(sizeof(AddDataResp)    == 64, "AddDataResp must be 64 bytes");
static_assert(sizeof(DeleteDataResp) == 6,  "DeleteDataResp must be 6 bytes");

// ---- Turn a structure into raw bytes, and back again ----
// We write one pair of functions per structure (no templates, kept simple).

inline QByteArray toBytes(const AddDataCmd &s) {
    return QByteArray(reinterpret_cast<const char *>(&s), sizeof(AddDataCmd));
}
inline QByteArray toBytes(const DeleteDataCmd &s) {
    return QByteArray(reinterpret_cast<const char *>(&s), sizeof(DeleteDataCmd));
}
inline QByteArray toBytes(const AddDataResp &s) {
    return QByteArray(reinterpret_cast<const char *>(&s), sizeof(AddDataResp));
}
inline QByteArray toBytes(const DeleteDataResp &s) {
    return QByteArray(reinterpret_cast<const char *>(&s), sizeof(DeleteDataResp));
}

inline bool fromBytes(const QByteArray &b, AddDataCmd &out) {
    if (b.size() != (int)sizeof(AddDataCmd)) return false;
    memcpy(&out, b.constData(), sizeof(AddDataCmd));
    return true;
}
inline bool fromBytes(const QByteArray &b, DeleteDataCmd &out) {
    if (b.size() != (int)sizeof(DeleteDataCmd)) return false;
    memcpy(&out, b.constData(), sizeof(DeleteDataCmd));
    return true;
}
inline bool fromBytes(const QByteArray &b, AddDataResp &out) {
    if (b.size() != (int)sizeof(AddDataResp)) return false;
    memcpy(&out, b.constData(), sizeof(AddDataResp));
    return true;
}
inline bool fromBytes(const QByteArray &b, DeleteDataResp &out) {
    if (b.size() != (int)sizeof(DeleteDataResp)) return false;
    memcpy(&out, b.constData(), sizeof(DeleteDataResp));
    return true;
}

// Copies a QString into the fixed 50-byte comment field.
inline void setComment(char dest[COMMENT_LENGTH], const QString &text) {
    memset(dest, 0, COMMENT_LENGTH);
    QByteArray bytes = text.toUtf8();
    int len = bytes.size();
    if (len > COMMENT_LENGTH - 1)
        len = COMMENT_LENGTH - 1;
    memcpy(dest, bytes.constData(), len);
}

// Reads the comment field back into a QString. Relies on it being
// zero-terminated somewhere inside the 50 bytes (setComment guarantees this).
inline QString commentToString(const char src[COMMENT_LENGTH]) {
    return QString::fromUtf8(src);
}

// Given the first byte of a command, returns how many bytes that whole
// command takes up. Returns 0 if the byte is not a command we know.
inline int commandSize(unsigned char cmdId) {
    if (cmdId == CMD_ADD || cmdId == CMD_UPDATE)
        return (int)sizeof(AddDataCmd);
    if (cmdId == CMD_DELETE)
        return (int)sizeof(DeleteDataCmd);
    return 0;
}
