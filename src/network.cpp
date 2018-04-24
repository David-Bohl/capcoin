#include "../lib/socket.h"
#include "../lib/network.h"
#include <iostream>
#include <vector>
#include <string>
#include <thread>

using namespace std;

void Network::broadcastMessage(string msg){
  send(sock, msg.c_str(), msg.size(), 0);
}


void Network::broadcastBlock(Block& block){

  Serialize serializer(block);

  string str = serializer.toString();

  send(sock, str.c_str(), str.size(), 0);

}

void Network::sendChain(int to, Blockchain* bc)
{
  vector<Block> chain = bc->GetChain();
  for(auto block: chain)
  {
    Serialize serializer(block);
    string blockStr = serializer.toString();
    server.broadcastToOne(to, blockStr);
  }
  server.broadcastToOne(to, "END");
}


std::string Network::getLastReceived(){
  return lastReceived;
}

void Network::listen(){
  vector<Block> blocks;

  while(1){
    FD_ZERO(&readfds);

    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sock, &readfds);

    // could probably remove this and just use sock
    max_sd = (sock > 0) ? sock : 0;

    activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);

    if((activity < 0) && (errno != EINTR)){
      cout << "\nSelect error\n";
    }

    // receive message
    if(FD_ISSET(sock, &readfds)){
      valread = read(sock, buffer, 1024);
      if(valread == 0){
        close(sock);
        cout << "\nConnection closed by host\n";
        //exit(0);
      }
      buffer[valread] = '\0';
      // this line below prints out any message
      //cout << string(buffer) << endl;

      string s = string(buffer);
      if(s.substr(1, 5) == "BLOCK")
      {
        //
        Block block = JSONtoBlock(s);

        // If blockchain is empty or just a genesis block then we will get every block before updating the chain
        if(blockchain == nullptr || blockchain->GetChain().size() == 1){
          blocks.push_back(block);
        }
        // Else it's just a new block
        else{
          blockchain->Push(block);
        }

      }

      if(s.substr(0, 3) == "END")
      {
        Blockchain bc(blocks);
        blockchain = &bc;
      }
      lastReceived = s;
      strcpy(buffer, "");
      fflush(stdout);
    }
  }
}

void Network::startClient(Blockchain * bc){

  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    cout << "\n Socket creation error \n";
    return;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(client.getLocalPort());

  if(inet_pton(AF_INET, ip_addr.c_str(), &serv_addr.sin_addr) <= 0){
    cout << "\nInvalid address\n";
    return;
  }

  if(connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    cout << "\nConnection Failed \n";
    return;
  }

  blockchain = bc;
}

void Network::runServer(Blockchain * bc) {

  TCPSocket* s;

  while(1)
  {

      //clear the socket set
      FD_ZERO(&readfds);

      //add master socket to set
      FD_SET(master_socket, &readfds);
      max_sd = master_socket;


      for(i=0; i < server.client_sockets.size(); i++)
      {
        s = server.client_sockets[i];

        //if valid socket descriptor then continue
        if(s == nullptr)
          continue;

        sd = s->getSockDesc();
        FD_SET(sd, &readfds);
        max_sd = sd;

      }

      //wait for an activity on one of the sockets , timeout is NULL ,
      //so wait indefinitely
      activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

      if ((activity < 0) && (errno!=EINTR))
      {
          // printf("select error");
          cout << "select error" << endl;
      }

      //If something happened on the master socket ,
      //then its an incoming connection
      if (FD_ISSET(master_socket, &readfds))
      {

          try {
            client_socket = server.accept();
          }
          catch(SocketException &e){
            cout << e.what() << endl;
            continue;
          }

          sd = client_socket->getSockDesc();

          for (i = 0; i < server.client_sockets.size(); i++)
          {
              //if position is empty
              if( server.client_sockets[i] == nullptr )
              {
                  server.client_sockets[i] = client_socket;
                  break;
              }
          }
          if(i == server.client_sockets.size())
            server.client_sockets.push_back(client_socket);
          cout << "Adding to list of sockets as " << i << endl;
          cout << "Socket descriptor is " << sd << endl;


          //send new connection greeting message
          if( send(sd, message, strlen(message), 0) != strlen(message) )
          {
              perror("send");
          }

          puts("Welcome message sent successfully");
      }


      for(i = 0; i<server.client_sockets.size(); i++)
      {
        s = server.client_sockets[i];
        if(s == nullptr){
          continue;
        }

        sd = s->getSockDesc();

          if (FD_ISSET( sd , &readfds))
          {
            //Check if it was for closing , and also read the
            //incoming message
            if ((valread = read( sd , buffer, 1024)) == 0)
            {
                // maybe add index to reduce time complexity
                server.closeConnection(sd);
                cout << "Connection " << sd << " closed" << endl;
            }

            //Handle Incoming Messages
            else
            {
                //set the string terminating NULL byte on the end of the data read
                buffer[valread] = '\0';

                string s = string(buffer);

                // if incoming message is REQUEST send out the chain
                if(s == "REQUEST"){


                  // Block block = bc->GetLastBlock();
                  //
                  // Serialize serializer(block);
                  //
                  // string blockStr = serializer.toString();
                  //
                  // server.broadcastToOne(sd, blockStr);

                  sendChain(sd, bc);

                }
                //if the incoming message is a new block
                else if(s.substr(1, 5) == "BLOCK"){

                  // Parse block
                  Block block = JSONtoBlock(s);

                  // Push
                  bc->Push(block);

                }
            		else{
            		  server.broadcastAll(sd, string(buffer));
            		}

            		// print out all incoming messages
                cout << string(buffer) << endl;

                strcpy(buffer, "");
            }
          }
      }
  }

}
