#ifndef NETWORK_H
#define NETWORK_H

#include "socket.h"
#include "block.h"
#include "blockchain.h"
#include "serialize.h"
#include <string>
#include <thread>


class Network{

public:
  void startClient(Blockchain * bc);
  void runServer(Blockchain * bc);

  void listen();
  std::thread listenThread(){
    return std::thread([=] { listen(); });
  }

  void broadcastMessage(std::string msg);

  void broadcastBlock(Block& block);

  void sendChain(int to, Blockchain* bc);

  std::string getLastReceived();

private:
  TCPClientSocket client{1025};
  TCPServerSocket server{1025, 5};
  int sock = 0, valread, activity, max_sd, sd, i;
  int master_socket = server.getSockDesc();
  struct sockaddr_in serv_addr;
  struct sockaddr_in address;
  std::string ip_addr =  "167.99.12.102";
  // std::string ip_addr =  "127.0.0.1";
  std::string strMessage = "Established Network Connection \r\n";
  const char *message = strMessage.c_str();
  TCPSocket* client_socket;
  char buffer[1025];
  fd_set readfds;
  std::string lastReceived = "";

  Blockchain * blockchain = nullptr;
};

#endif
