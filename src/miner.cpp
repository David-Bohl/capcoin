#include "miner.h"

Miner::Miner(Blockchain* chain, TransactionPool* txpool, UnspentTxOutPool* utxoutpool, Network* nw, bool* killMiner,  std::string address):chain_{chain}, txpool_{txpool}, utxopool_{utxoutpool}, nw_{nw}, killMiner_{killMiner}, address_{address}{};


void Miner::mine_loop() {
    std::cout << "[miner]: Miner listener activated." << std::endl;
    while(true) {
        //Create a coinbase transaction for the miner, aka the reward
        TxIn coinbaseIn("","",chain_->GetHeight());
        TxOut coinbaseOut(address_, 50);
        std::vector<TxIn> ins{coinbaseIn};
        std::vector<TxOut> outs{coinbaseOut};
        Transaction coinbaseTx{ins,outs};

        //Collection of transactions that will be placed in the mined block
        std::vector<Transaction> txSupply{coinbaseTx};

        //A flag signaling whether mining was successful
        bool success = false;

        //start a timer
        time_t start = time(0);


        //This loop packages transaction for the block
        //When packaging is completed, mining will commence
        //Loop runs when the signal to kill miner is off.
        //Exits when mining fails
        while(!*killMiner_ && !success){
            //if > 20 txns in pool, package 20 and generate a block
            //if 20 > n > 0 txns in pool, and 200 seconds pass, package
            //however many there are into a block.
            int size = txpool_->size() > 20 ? 20 : txpool_->size();
            if(size == 20 || time(0)-30 > start){
                  std::cout << "[miner-test]: entered, txpool_size: " << size << std::endl;
                  //Package transactions into txSupply
                  for(int i = 0; i < size; i++){
                    txSupply.push_back(txpool_->front());
                    txpool_->pop();
                  }
                  
                  // Retrieve current size
                  size_t beforeMiningHeight = chain_->GetHeight(); 

                  // Mine the block
                  success = chain_->GenerateNextBlock(killMiner_, txSupply);
                  if(success) {
                      std::cout << "[miner-succ]: Chain Height: " << chain_->GetHeight();
                      std::cout << " Before Height: " << beforeMiningHeight << std::endl;
                      if(chain_->GetHeight() == beforeMiningHeight+1) {
                        //Broadcast block to network
                          std::cout << "[miner-succ]: Telling the network to broadcast block" << std::endl;
                          Block b = chain_->GetLastBlock();
                          std::cout << "[miner-succ]: block tx size: " << b.GetData().size() << std::endl;
                          std::cout << "[miner-succ]: block index:" << b.GetIndex() << std::endl;
                          nw_->broadcastBlock(b);
                          start = time(0);

                          //empty txSupply
                          txSupply.clear();
                      }
                      else {
                        // if block was rejected, dump it's contents
                        chain_->Dump();
                        std::cout << "[miner-fail]: Block dumped" << std::endl;
                      }
                  }
            }
        }
        //If there was a signal to kill the miner and there are transactions
        //from the pool in the packaged transactions
        //put packaged transaction back into pool
        if(*killMiner_ && txSupply.size() > 1) {
            std::cout << "[miner]: Putting back transactions into mempool." << std::endl;
            // push back all but the coinbase transaction
            for(int i = txSupply.size()-1; i > 0; i--) {
                txpool_->push(txSupply[i]);
            }
            std::cout << "[miner]: All transactions have been placed back into mempool.";
            std::cout << std::endl;
        }

      *killMiner_ = false;
    }

}

std::thread Miner::mineThread() {
    return std::thread([=] {
                mine_loop();
            });
}
