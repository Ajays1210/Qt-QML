QT TASK - COMPLETE STARTER PROJECT

This package contains two separate Qt/C++ applications:
1. Server - TCP 4001 receiver + UDP 4002 broadcaster.
2. Client - Qt Widgets GUI with Add, Update and Delete.

ASSUMPTIONS:
- cmdID: 0x01 Add, 0x02 Update, 0x03 Delete
- respID: 0x81 Add, 0x82 Update, 0x83 Delete
- Ack: 0x00 = Fail, 0xFF = Success
- Update command/response use the same fields as Add.
- Binary structures are packed to avoid compiler padding.
- Server keeps its data in memory only; restarting the server clears it.
- For a local demo, run Server first, then Client.
- Client uses localhost (127.0.0.1). Change SERVER_IP in client/networkworker.h if needed.
- UDP responses are broadcast to port 4002.

BUILD:
Open each folder separately in Qt Creator. These projects use CMake and Qt 6.
Select a Qt 6 kit, configure, build and run.

RUN ORDER:
1. Build/run Server.
2. Build/run Client.
3. Enter Unique ID, Latitude, Longitude and Comment.
4. Add -> Apply.
5. Select a row -> Update -> edit -> Apply.
6. Select a row -> Delete -> confirm.

IMPORTANT:
The assignment wording says GUI is in the main thread and sending/reception should be
in different threads. The client uses one QThread for network operations, while the
GUI remains in the main thread. The server has separate TCP and UDP worker threads.

This is intentionally kept simple so you can explain it in an interview/review.
