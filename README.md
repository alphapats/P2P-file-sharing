# P2P-file-sharing

In this project,a simple 'Napster' like P2P system for file search and retrieval has been implemented. A user (peer) joins a
P2P system by contacting a central server (server). The peer publishes the files she wishes to share by
specifying the same to the central server. The peer searches for a file, again by contacting the central
server. Based on input from the server, peer fetches the file from another peer (who has the file).

1. There is a single central server (server.c) and at least 4 peers. Peers are different instances of
the same code client.c. The server should be able to handle multiple peers simultaneously.

2. All peers know the central server details. This can be accomplished via a command-line
argument. Example: ./client server-ip server-port

4. Each peer also has its own local folder named 'p2p-files' which is used to store files it wishes
to publish as well as ones which it fetches from others.

5. Each peer supports duplex operation i.e. it should be able to transmit (upload to another
peer) as well as receive files (download from another peer) simultaneously. Multiple uploads
and downloads are also be supported.

6. Code works over any type of file: pdfs, jpg, png, txt etc
