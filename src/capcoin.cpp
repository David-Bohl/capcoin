#include "fullNode.h"
#include "network.h"
#include "block.h"
#include "blockchain.h"
#include "wallet.h"
#include "utxoutpool.h"
#include "transactionpool.h"
#include "miner.h"

#include <string.h>
#include <iostream>
#include <thread>
#include <pthread.h>

using namespace std;


int main(int argc, char *argv[]) {

  bool killMinerSignal = false;
  if((argc > 1) && (strncmp (argv[1], "server", 6) == 0)){
    //server
    std::cout << "[node]: Starting as server" << std::endl;

    // create Transaction Pool
    TransactionPool txpool;

    // create UTxOutPool
    UnspentTxOutPool utxoutpool;

    // create Blockchain
    Blockchain bc(&txpool, &utxoutpool);
    // Block genBlock = bc.GetLastBlock();

    //Test input data for generating a new block
    TxIn in1("a", "asdasdasd", 1);
    TxOut out1("b", 50);

    TxIn in2("c", "asdfasdfasdf", 2);
    TxOut out2("d", 100);

    TxIn in3("e", "asdfadf", 3);
    TxOut out3("f", 150);

    std::vector<TxIn> ins{in1, in2, in3};
    std::vector<TxOut> outs{out1, out2, out3};
    Transaction first(ins, outs);
    std::vector<Transaction> GenTxns{first};

    killMinerSignal = true;
    std::cout << "[node]: Generating a new block." << std::endl;
    bc.GenerateNextBlock(&killMinerSignal, GenTxns);

    //create Network
    Network nw(&bc, &txpool, &utxoutpool, &killMinerSignal);
    nw.runServer();

    //  network listen on seperate thread


    //FullNode node (&bc, &nw);
    // node.run();


  }else{
    //client

    // first, create all 4 parts of the node, Blockchain, Network, Miner and Wallet

    // create Transaction Pool
    TransactionPool txpool;

    // create UTxOutPool
    UnspentTxOutPool utxoutpool;

    // create Blockchain
    Blockchain bc(&txpool, &utxoutpool);
    Block genBlock = bc.GetLastBlock();

    // create Network
    Network nw(&bc, &txpool, &utxoutpool, &killMinerSignal);
    //connect as client
    nw.startClient();
    //start listening for incoming messages, on another thread
    std::thread listenThread = nw.listenThread();


    // create Wallet
    Wallet wa(&txpool, &utxoutpool);


    std::string address = wa.GetAddress();
    // create Miner
    Miner miner (&bc,&txpool,&utxoutpool,&nw,&killMinerSignal,address);
    std::thread miner_thread = miner.mineThread();

    double amt = 2222222;
    TxIn dummyIn("", "", 0);
    TxOut dummyOut("22222222222222222222222222222222", amt);
    std::vector<TxIn> TxIns{dummyIn};
    std::vector<TxOut> TxOuts{dummyOut};

    Transaction dummy = {TxIns, TxOuts};

    double amt2 = 3333333;
    TxIn dummyIn2("", "", 0);
    TxOut dummyOut2("33333333333333333333333333333333", amt2);
    std::vector<TxIn> TxIns2{dummyIn2};
    std::vector<TxOut> TxOuts2{dummyOut2};

    Transaction dummy2 = {TxIns2, TxOuts2};


    txpool.push(dummy);
    txpool.push(dummy2);
    // Initalize Full Node with:
    // Blockchain, Network, Wallet, Miner
    // TransactionPool
    FullNode node (&bc, &nw, &wa, &txpool, &killMinerSignal);

    node.updateChain();
    node.welcome();

    // start the node
    node.run();
  }




    return 0;
}
