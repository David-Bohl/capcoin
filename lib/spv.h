#include <iostream>
#include <sstream>

class SPV_TXN {
public:
  SPV_TXN(std::string&, std::string&, std::string&, double);
  std::string private_key, public_key;
  std::string to_address;
  double amount;
};

SPV_TXN JSONtoSPV(const std::string& txnString);
