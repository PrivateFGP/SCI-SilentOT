/*
Original Author: CBackyx
*/

#ifndef UTILS_FHE_H__
#define UTILS_FHE_H__

#include "LinearHE/defines-HE.h"
#include "seal/seal.h"
#include "utils/emp-tool.h"

class FHEUtil {
public:
  seal::SEALContext *context;
  seal::Encryptor *encryptor;
  seal::Decryptor *decryptor;
  seal::Evaluator *evaluator;
  seal::BatchEncoder *encoder;
  seal::PublicKey *pub_key;
  seal::SecretKey *sec_key;
  seal::GaloisKeys *gal_keys;
  seal::Ciphertext *zero;
  size_t slot_count;

  uint64_t pk_size;
  uint64_t gk_size;
  uint64_t sk_size;

  bool has_sec_key;
  // ConvMetadata data;

  FHEUtil() {
    context = NULL;
    encryptor = NULL;
    decryptor = NULL;
    evaluator = NULL;
    encoder = NULL;
    pub_key = NULL;
    sec_key = NULL;
    gal_keys = NULL;
    zero = NULL;
    slot_count = 0;
    pk_size = 0;
    gk_size = 0;
    sk_size = 0;
    has_sec_key = false;    
  }

  FHEUtil(const FHEUtil& fu) {
    context = fu.context;
    encryptor = fu.encryptor;
    decryptor = fu.decryptor;
    evaluator = fu.evaluator;
    encoder = fu.encoder;
    pub_key = fu.pub_key;
    sec_key = fu.sec_key;
    gal_keys = fu.gal_keys;
    zero = fu.zero;
    slot_count = fu.slot_count;
    pk_size = fu.pk_size;
    gk_size = fu.gk_size;
    sk_size = fu.sk_size;
    has_sec_key = fu.has_sec_key;
  }

  FHEUtil& operator=(const FHEUtil& fu) {
    context = fu.context;
    encryptor = fu.encryptor;
    decryptor = fu.decryptor;
    evaluator = fu.evaluator;
    encoder = fu.encoder;
    pub_key = fu.pub_key;
    sec_key = fu.sec_key;
    gal_keys = fu.gal_keys;
    zero = fu.zero;
    slot_count = fu.slot_count;
    pk_size = fu.pk_size;
    gk_size = fu.gk_size;
    sk_size = fu.sk_size;
    has_sec_key = fu.has_sec_key;

    return *this;    
  }

  void setup_keys();

  void free_keys();

  void encrypt(std::vector<seal::Ciphertext> &ct,
                std::vector<std::vector<uint64_t>> &pt);
  void encrypt(seal::Ciphertext &ct,
                std::vector<uint64_t> &pt);

  void decrypt(std::vector<seal::Ciphertext> &ct,
                std::vector<std::vector<uint64_t>> &pt);
  void decrypt(seal::Ciphertext &ct,
                std::vector<uint64_t> &pt);

  void serialize_pub_key(uint8_t *buf) const;
  void deserialize_pub_key(uint8_t *buf, size_t pk_size_, size_t gk_size_);

  ~FHEUtil() {}
};

#endif
