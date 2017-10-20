# Socks
C++ Sockets

Server code:
Single thread loop that listens over tcp socket.
Utilizes descriptors to store connections.
Accepts incoming connections through select.
Recvs incoming messages through select.

Features:
-Is non-blocking    

-Utilizes Cereal for serialization of structure for messages. (http://uscilab.github.io/cereal/)


Client Code:
Done. Connects to inputted server(no checking) on port 5001(magic number)



