UnspentTxOutPool:: UnspentTxOutPool(){}

UnspentTxOut* UnspentTxOutPool:: FindFromIn(const TxIn& input){
  UnspentTxOut* outptr = nullptr;
  for(UnspentTxOut out: uTxOuts_){
    if (out.txOutId_ == input.id_ && out.txOutIndex_ == input.index_)
      outptr = out;
  }
  return outptr;
}

bool UnspentTxOutPool:: AddTxn(UnspentTxOut& input){
  uTxOuts_.push_back(input);
  return true;
}
