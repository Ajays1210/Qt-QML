# QtTaskClient (QML)

This is a self-contained client project that talks to your existing
`QtTaskServer` project, unchanged.

## Important: protocol.h must stay identical on both sides

Your server uses `#pragma pack(push, 1)` raw structs sent with
`reinterpret_cast<const char*>(&struct)` + `memcpy` — there's no
byte-order/serialization layer, so the client's `protocol.h` must be a
byte-for-byte copy of the server's. This folder already contains an exact
copy. **If you ever edit the server's `protocol.h`, copy the same change
into the client's `protocol.h` too**, or the two sides will silently
misread each other's structs.

## Project layout

```
Client/
  CMakeLists.txt
  protocol.h        <- exact copy of the server's protocol.h
  ItemModel.*        <- list model bound to the QML ListView
  NetworkClient.*     <- object QML calls (sendAdd/sendUpdate/sendDelete)
  TcpSender.*         <- own thread, writes raw command bytes to TCP:4001
  UdpReceiver.*       <- own thread, reads broadcast responses from UDP:4002
  main.cpp
  qml/Main.qml        <- the GUI: list + Add/Update/Delete + form popup
```

## How to open/build in Qt Creator

1. Put this `Client` folder next to (or wherever you like relative to) your
   `QtTaskServer` folder — they're independent CMake projects.
2. In Qt Creator: **File > Open File or Project** → select
   `Client/CMakeLists.txt`.
3. Pick a Qt6 kit (needs Qt6 Quick + Network) and configure.
4. Build (Ctrl+B) — this produces the `QtTaskClient` executable.

## How to run it

1. Run your existing `QtTaskServer` first (it logs
   `Server listening on TCP port 4001`).
2. Run `QtTaskClient`. You can start more than one instance — they'll all
   receive the same UDP broadcast.
3. **Add**: fill in Unique ID / Latitude / Longitude / Comment, click
   **Apply** — this builds an `AddDataCmd`, sends it over TCP, and the row
   appears in the list once the server's `AddDataResp` (Ack = success)
   arrives over UDP.
4. Click a row to select it, then **Update** (form opens pre-filled, ID
   locked) or **Delete**.

## What matches your server exactly

- Command IDs: `CMD_ADD=0x01`, `CMD_UPDATE=0x02`, `CMD_DELETE=0x03`
- Response IDs: `RESP_ADD=0x81`, `RESP_UPDATE=0x82`, `RESP_DELETE=0x83`
- Struct field names/order/packing: `cmdID/respID, UniqueID, lat, longi,
  comment[50]` (or the shorter Delete variants), all `#pragma pack(1)`
- Fixed sizes checked against your `static_assert`s: `AddDataCmd`=63,
  `AddDataResp`=64, `DeleteDataCmd`=5, `DeleteDataResp`=6
- Same ports: TCP 4001 (commands), UDP 4002 (broadcast responses)

## Notes

- The comment field is copied with `memset` + `memcpy` on the way out (so
  it's always zero-padded and null-terminated) and read back stopping at
  the first `'\0'` on the way in — safer than assuming it's always
  terminated.
- `TcpSender` connects to `QHostAddress::LocalHost`. If you run the server
  on a different machine, change that to the server's IP in
  `TcpSender.cpp`.
