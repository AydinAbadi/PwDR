//author: Aydin Abadi
#include <iostream>
#include <math.h>
#include <string>
#include <stdio.h>
#include <vector>
#include "gmp.h"
#include <gmpxx.h>
#include "Rand.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/sha.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include "cryptopp/secblock.h"
#include "cryptopp/osrng.h"
#include "cryptopp/cryptlib.h"
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::CTR_Mode;
using namespace CryptoPP;
using namespace std;
typedef mpz_t bigint;
typedef unsigned char byte;


// description: Generates a pseudorandom value (by  encrypting "val" given the key and initial vector).
bigint* encrypt(int val, byte* key, int key_size, byte* iv, int byte_){

  string cipher, temp;
  CBC_Mode< AES >::Encryption e;
  bigint* res;
  res = (bigint*)malloc(1 * sizeof(mpz_t));
  unsigned char prn_[byte_];
  e.SetKeyWithIV(key, key_size, iv);
  StringSource sss(to_string(val), true, new StreamTransformationFilter(e, new StringSink(cipher)));
  temp = cipher.substr (0, byte_);// truncate the ciphertext
  memset(prn_, 0x00, byte_ + 1);
  strcpy((char*)prn_, temp.c_str());
  mpz_init(res[0]);
  mpz_import(res[0], byte_, 1, 1, 0, 0, prn_);
  return res;
}

//description: Private Verdict Encoding (PVE) algorithm.
bigint* PVE(byte* key, int key_size, byte* iv, int w, int n, int j){

  bigint* res,  *temp_1;
  Random rd_;
  res = (bigint*)malloc(1 * sizeof(mpz_t));
  temp_1 = (bigint*)malloc(1 * sizeof(mpz_t));
  mpz_init_set_str(res[0], "0", 10);
  //generate a masking factor
  if(j < n){
    res = encrypt(j, key, key_size, iv, 16);
    // cout<<"\n in j<n --res[0]: "<<res[0]<<endl;
  }
  else if(j == n){
    for(int i = 1; i < n; i++){
      bigint *temp_2 = encrypt(i, key, key_size, iv, 16);
      // cout<<"\n in j==n --temp_2[0]: "<<temp_2[0]<<endl;
      mpz_xor(res[0], res[0], temp_2[0]);
    }
  }
  //represent the verdict by 0 or a random value
  if(w == 0){
    mpz_init_set_str(temp_1[0], "0", 10);
  }
  else{
    temp_1 = rd_.gen_randSet(1, 128);
  }
  // mask the verdict's representation
  mpz_xor(res[0], res[0], temp_1[0]);
  return res;
}

//description:: Final Verdict Decoding (FVD) Algorithm
int FVD(int n, bigint* w){
  bigint temp, zero;
  mpz_init_set_str(temp, "0", 10);
  mpz_init_set_str(zero, "0", 10);
  //combine all verdicts' representations
  for(int i = 1; i < n + 1; i++){
    mpz_xor(temp, temp, w[i]);
  }
  //extract the final verdict
  if(mpz_cmp(temp, zero) == 0){
    return 0;
  }
  else{
    return 1;
  }
}


int main(){

  //---gen PRF key
  byte key[AES::DEFAULT_KEYLENGTH];
  int key_size = AES::DEFAULT_KEYLENGTH;
  byte iv[AES::BLOCKSIZE];
  AutoSeededRandomPool prng;
	prng.GenerateBlock(key, key_size);// seed_: master seed for PRF
  //----------------
  int n = 10;
  bigint* w_, *temp;
  w_ = (bigint*)malloc(n+1 * sizeof(mpz_t));
  int w_1 = 0;
  int w_2 = 0;
  int w[n];
  for (int i = 1; i < n + 1; i++){
    w[i] = 0;
  }
  for (int i = 1; i < n + 1; i++){
      temp = PVE(key, key_size, iv, w[i], n, i);
      cout<<"\n temp[0]: "<<temp[0]<<endl;
      mpz_init_set(w_[i], temp[0]);
  }
  int res = FVD(n, w_);
  cout<<"\n res: "<<res<<endl;


}


//g++ -I /Users/aydinabadi/desktop/c++-test/cryptopp Rand.o  main.cpp /Users/aydinabadi/desktop/c++-test/cryptopp/libcryptopp.a  -o main -lgmpxx -lgmp
//g++ -I /Users/aydinabadi/desktop/c++-test/cryptopp Rand.o  modified-merkle-tree.cpp /Users/aydinabadi/desktop/c++-test/cryptopp/libcryptopp.a  -o main -lgmpxx -lgmp