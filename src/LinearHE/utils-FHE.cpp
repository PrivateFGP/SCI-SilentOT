#include "LinearHE/utils-FHE.h"
#include "seal/util/polyarithsmallmod.h"

using namespace std;
using namespace sci;
using namespace seal;
using namespace seal::util;

const std::map<int32_t, uint64_t> fhe_default_prime_mod{
    {32, 4293918721},    {33, 8585084929},   {34, 17171218433},
    {35, 34359214081},   {36, 68686184449},  {37, 137352314881},
    {38, 274824036353},  {39, 549753716737}, {40, 1099480956929},
    {41, 2198100901889},
};

void FHEUtil::setup_keys() {
    this->slot_count = POLY_MOD_DEGREE;

    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(this->slot_count);
    parms.set_coeff_modulus(CoeffModulus::Create(this->slot_count, {60, 60, 60, 38}));
    printf("bit length %d\n", bitlength);
    // printf("prime_mod %lld\n", prime_mod);
    parms.set_plain_modulus(fhe_default_prime_mod.at(bitlength));
    // auto context = SEALContext::Create(parms, true, sec_level_type::none);
    this->context = new SEALContext(parms, true, sec_level_type::none);
    this->encoder = new BatchEncoder(*context);
    this->evaluator = new Evaluator(*context);

    KeyGenerator keygen(*(this->context));
    auto sec_key_ = keygen.secret_key();
    auto pub_key_se = keygen.create_public_key();
    auto gal_keys_se = keygen.create_galois_keys();

    stringstream os;
    pub_key_se.save(os);
    this->pk_size = os.tellp();
    gal_keys_se.save(os);
    this->gk_size = (uint64_t)os.tellp() - this->pk_size;
    sec_key_.save(os);
    this->sk_size = (uint64_t)os.tellp() - this->pk_size - this->gk_size;

    PublicKey pub_key_;
    pub_key_.load(*this->context, os);
    GaloisKeys gal_keys_;
    gal_keys_.load(*this->context, os);

    this->encryptor = new Encryptor(*(this->context), pub_key_);
    this->decryptor = new Decryptor(*(this->context), sec_key_);
    this->gal_keys = new GaloisKeys(gal_keys_);
    this->pub_key = new PublicKey(pub_key_);
    this->sec_key = new SecretKey(sec_key_);

    this->zero = new Ciphertext();
    vector<uint64_t> pod_matrix(slot_count, 0ULL);
    Plaintext tmp;
    this->encoder->encode(pod_matrix, tmp);
    this->zero = new Ciphertext;
    this->encryptor->encrypt(tmp, *zero);
    
    this->has_sec_key = true;
}

void FHEUtil::free_keys() {
    delete this->encryptor;
    delete this->evaluator;
    delete this->encoder;
    delete this->pub_key;
    if (this->has_sec_key) {
        delete this->decryptor;
        delete this->sec_key;
    }
    delete this->gal_keys;
    delete this->zero;
    delete this->context;
}

void FHEUtil::serialize_pub_key(uint8_t *buf) const {
    stringstream os;
    this->pub_key->save(os);
    uint64_t pk_size_ = os.tellp();
    this->gal_keys->save(os);
    uint64_t gk_size_ = (uint64_t)os.tellp() - pk_size_;
    os.read((char*)buf, pk_size_ + gk_size_);
}

void FHEUtil::deserialize_pub_key(uint8_t *buf, size_t pk_size_, size_t gk_size_) {
    // Setup first
    this->slot_count = POLY_MOD_DEGREE;

    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(slot_count);
    parms.set_coeff_modulus(CoeffModulus::Create(slot_count, {60, 60, 60, 38}));
    parms.set_plain_modulus(fhe_default_prime_mod.at(bitlength));
    
    this->context = new SEALContext(parms, true, sec_level_type::none);
    this->encoder = new BatchEncoder(*context);
    this->evaluator = new Evaluator(*context);

    // Deserialize
    stringstream is;
    PublicKey pub_key_;
    is.write((char*)buf, pk_size_);
    pub_key_.load(*this->context, is);
    this->pub_key = new PublicKey(pub_key_);
    this->gal_keys = new GaloisKeys();
    is.write((char*)buf + pk_size_, gk_size_);
    this->gal_keys->load(*this->context, is);

    this->pk_size = pk_size_;
    this->gk_size = gk_size_;

    this->encryptor = new Encryptor(*this->context, pub_key_);
    vector<uint64_t> pod_matrix(slot_count, 0ULL);
    Plaintext tmp;
    this->encoder->encode(pod_matrix, tmp);
    this->zero = new Ciphertext;
    this->encryptor->encrypt(tmp, *zero);

    this->has_sec_key = false;    
}

void FHEUtil::encrypt(std::vector<seal::Ciphertext> &ct,
                      std::vector<std::vector<uint64_t>> &pt) {
  ct.clear();
  ct.resize(pt.size());
#pragma omp parallel for num_threads(num_threads) schedule(static)
  for (size_t ct_idx = 0; ct_idx < pt.size(); ct_idx++) {
    Plaintext tmp;
    // printf("pack-length %d\n", pt[ct_idx].size());
    this->encoder->encode(pt[ct_idx], tmp);
    this->encryptor->encrypt(tmp, ct[ct_idx]);
  }
}

void FHEUtil::decrypt(std::vector<seal::Ciphertext> &ct,
                      std::vector<std::vector<uint64_t>> &pt) {
  // Decrypt ciphertext
  pt.resize(ct.size());

#pragma omp parallel for num_threads(num_threads) schedule(static)
  for (int ct_idx = 0; ct_idx < ct.size(); ct_idx++) {
    Plaintext tmp;
    this->decryptor->decrypt(ct[ct_idx], tmp);
    this->encoder->decode(tmp, pt[ct_idx]);
  }
}

void FHEUtil::encrypt(seal::Ciphertext &ct,
                      std::vector<uint64_t> &pt) {
    Plaintext tmp;
    this->encoder->encode(pt, tmp);
    this->encryptor->encrypt(tmp, ct);
}

void FHEUtil::decrypt(seal::Ciphertext &ct,
                      std::vector<uint64_t> &pt) {
    Plaintext tmp;
    this->decryptor->decrypt(ct, tmp);
    this->encoder->decode(tmp, pt);
}