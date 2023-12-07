#include "FloatingPoint/fixed-point.h"
#include "utils/performance.h"
#include <random>
#include <limits>
#include <omp.h>

using namespace sci;
using namespace std;

#define SCALER_BIT_LENGTH 8

enum class Op { ADD, MUL, DIV, SQRT, CHEAP_ADD, CHEAP_DIV };

enum class CmpOp { LT, GT, GE, LE };

Op op = Op::DIV;
CmpOp cmp_op = CmpOp::LT;
bool verbose = true;
sci::NetIO *iopack = nullptr;
sci::OTPack<sci::NetIO> *otpack = nullptr;
FixOp *fix_op = nullptr;
int sz = 10;
int party = 1;
string address = "127.0.0.1";
int port = 8000;

void test_op() {
  assert(party == sci::ALICE || party == sci::BOB);
  FixArray fa;
  uint64_t *f_1 = new uint64_t[sz];
  uint64_t *f_2 = new uint64_t[sz];
  uint64_t *f = new uint64_t[sz];
  double *actual = new double[sz];
  for (int i = 0; i < sz; i++) {
    f_1[i] = (((uint64_t)i+1)) << 20;
    f_2[i] = (((uint64_t)i+1)*(i+1)) << 20;
  }
  // fa = fix_op->input(party, sz, f_1, false, 64);
  FixArray fa_1 = fix_op->input(sci::ALICE, sz, f_1, true, 40, 16);
  FixArray fa_2 = fix_op->input(sci::BOB, sz, f_2, false, 40, 16);
  switch (op) {
    // case Op::ADD:
    //     cout << "ADD" << endl;
    //     for (int i = 0; i < sz; i++) {
    //     f[i] = f_1[i] + f_2[i] + 1;
    //     }
    //     fa = fix_op->add(fa, 1);
    //     break;
    // case Op::MUL:
    //     cout << "MUL" << endl;
    //     for (int i = 0; i < sz; i++) {
    //     f[i] = f_1[i] * f_2[i];
    //     }
    //     fa = fix_op->mul(fa_1, fa_2);
    //     break;
    case Op::DIV:
        cout << "DIV" << endl;
        for (int i = 0; i < sz; i++) {
          printf("%llu %llu\n", f_1[i], f_2[i]);
          f[i] = f_1[i] / f_2[i];
          actual[i] = (double)f_1[i] / f_2[i];
        }
        fa = fix_op->div(fa_1, fa_2, 40, 16);
        break;
    default:
        assert(false);
  }
  FixArray fa_pub = fix_op->output(sci::PUBLIC, fa);
  vector<int64_t> f_ = fa_pub.get_native_type<int64_t>();
  for (int i = 0; i < sz; i++) {
    if (verbose) {
      FixArray fa_i = fa_pub.subset(i, i + 1);
      cout << i << "\t" << f_1[i] << "\t" << f_2[i] << "\t" << f[i] << "\t"
           << f_[i] << "\t" << actual[i] << "\t" << fa_i << endl;
    }
    // assert(f[i] == f_[i]);
  }
  delete[] f;
  delete[] f_1;
  delete[] f_2;
}

void test_cmp_op() {
  assert(party == sci::ALICE || party == sci::BOB);
  BoolArray bp;
  uint64_t *f_1 = new uint64_t[sz];
  uint64_t *f_2 = new uint64_t[sz];
  uint8_t *b = new uint8_t[sz];
  uint8_t *b_ = new uint8_t[sz];
  for (int i = 0; i < sz; i++) {
    f_1[i] = i;
    f_2[i] = i+1;
  }
  FixArray fa;
  FixArray fa1;
  FixArray fa2;
  FixArray fa3;
  FixArray fa_1 = fix_op->input(sci::ALICE, sz, f_1, true, 64);
  FixArray fa_2 = fix_op->input(sci::BOB, sz, f_2, true, 64);
  // printf("fa_1 party %d\n", fa_1.party);
  // printf("fa_2 party %d\n", fa_2.party);

  switch (cmp_op) {
  case CmpOp::LT:
    cout << "LT" << endl;
    for (int i = 0; i < sz; i++) {
      b[i] = f_1[i] < f_2[i];
    }
    bp = fix_op->LT(fa_1, fa_2);
    // fa = fix_op->add(fa_1, fa_2);
    fa = fix_op->if_else(bp, fa_1, fa_2);
    fa1 = fix_op->if_else(bp, fa_1, fa_2);
    // fa2 = fix_op->if_else(bp, fa_1, fa_2);
    // fa3 = fix_op->if_else(bp, fa_1, fa_2);
    break;
  case CmpOp::GT:
    cout << "GT" << endl;
    for (int i = 0; i < sz; i++) {
      b[i] = f_1[i] > f_2[i];
    }
    bp = fix_op->GT(fa_1, fa_2);
    break;
  case CmpOp::LE:
    cout << "LE" << endl;
    for (int i = 0; i < sz; i++) {
      b[i] = f_1[i] <= f_2[i];
    }
    bp = fix_op->LE(fa_1, fa_2);
    break;
  case CmpOp::GE:
    cout << "GE" << endl;
    for (int i = 0; i < sz; i++) {
      b[i] = f_1[i] >= f_2[i];
    }
    bp = fix_op->GE(fa_1, fa_2);
    break;
  default:
    assert(false);
  }
  // printf("Here\n");
  BoolArray bp_pub = fix_op->bool_op->output(sci::PUBLIC, bp);
  memcpy(b_, bp_pub.data, sz * sizeof(uint8_t));
  for (int i = 0; i < sz; i++) {
    uint32_t f_int_1 = *((uint32_t *)&f_1[i]);
    uint32_t f_int_2 = *((uint32_t *)&f_2[i]);
    if (verbose) {
      cout << i << "\t" << f_1[i] << "\t" << f_2[i] << "\t" << int(b[i]) << "\t"
           << int(b_[i]) << endl;
    }
    assert(b[i] == b_[i]);
  }
  delete[] b;
  delete[] b_;
  delete[] f_1;
  delete[] f_2;
}

template <typename T>
void print_vector_of_vector(const std::vector<std::vector<T>>& v, size_t lines = 0) {
  size_t cnt = 0;
  // Loop through each vector in the vector of vector
  for (auto vec : v) {
    if (lines != 0 && cnt >= lines) break;
    // Loop through each element in the vector
    for (auto elem : vec) {
      // Print the element with a space
      std::cout << elem << " ";
    }
    // Print a newline after each vector
    std::cout << "\n";
    cnt++;
  }
}

template <typename T>
void print_vector(const std::vector<T>& v, size_t num = 0) {
  size_t cnt = 0;
  for (auto elem : v) {
    if (num != 0 && cnt >= num) break;
    std::cout << elem << " ";
    cnt++;
  }
  std::cout << "\n";
}

// Matrix multiplication of A and B
std::vector<std::vector<double>> matmul(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B) {
  // Get the number of rows and columns of the matrices
  int n_rows = A.size();
  int n_cols = B[0].size();
  int n_inner = A[0].size();

  // Initialize the output matrix C as a zero matrix
  std::vector<std::vector<double>> C(n_rows, std::vector<double>(n_cols, 0.0));

  // Set the number of threads for OMP
  int n_threads = 32; // You can change this according to your system
  omp_set_num_threads(n_threads);

  // Parallelize the matrix multiplication with OMP
  #pragma omp parallel for
  for (int i = 0; i < n_rows; i++) {
    for (int j = 0; j < n_cols; j++) {
      // Loop over the inner dimension of the matrices
      for (int k = 0; k < n_inner; k++) {
        // Add the product of the corresponding elements of A and B to C
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }

  // Return the output matrix C
  return C;
}

void test_matrix_multiplication() {
  std::srand(42);
  size_t dim0 = 4;
  size_t dim1 = 4;
  size_t dim2 = 4;
  std::vector<std::vector<double>> A(dim0, std::vector<double>(dim1, 0.0)); 
  std::vector<std::vector<double>> B(dim1, std::vector<double>(dim2, 0.0)); 
  std::vector<std::vector<uint64_t>> encoded_A(dim0, std::vector<uint64_t>(dim1, 0)); 
  std::vector<std::vector<uint64_t>> encoded_B(dim1, std::vector<uint64_t>(dim2, 0));
  std::vector<std::vector<double>> C(dim0, std::vector<double>(dim2, 0.0)); 
  if (party == sci::ALICE) {
    for (size_t i = 0; i < dim0; ++i) {
      for (size_t j = 0; j < dim1; ++j) {
        A[i][j] = (double) std::rand() / RAND_MAX;
        encoded_A[i][j] = (int64_t)(A[i][j] * (1 << SCALER_BIT_LENGTH));
      }
    }
    for (size_t i = 0; i < dim1; ++i) {
      for (size_t j = 0; j < dim2; ++j) {
        B[i][j] = (double) std::rand() / RAND_MAX;
        encoded_B[i][j] = (int64_t)(B[i][j] * (1 << SCALER_BIT_LENGTH));
      }
    }
  }
  C = matmul(A, B);

  std::vector<FixArray> sci_A(dim0);
  std::vector<FixArray> sci_B(dim1);
  std::vector<FixArray> sci_C(dim0);
  for (size_t i = 0; i < dim0; ++i) {
    sci_A[i] = fix_op->input(party, dim1, (uint64_t*)&(encoded_A[i][0]), true, 64, SCALER_BIT_LENGTH);
    sci_A[i] = fix_op->reduce(sci_A[i], 32);
  }
  for (size_t i = 0; i < dim1; ++i) {
    sci_B[i] = fix_op->input(party, dim2, (uint64_t*)&(encoded_B[i][0]), true, 64, SCALER_BIT_LENGTH);
    sci_B[i] = fix_op->reduce(sci_B[i], 32);
  }

  sci_C = fix_op->mul(sci_A, sci_B, 32);

  for (size_t i = 0; i < dim0; ++i) {
    sci_C[i] = fix_op->extend(sci_C[i], 64);
  }

  fix_op->print(sci_C);

  printf("---------\n");

  print_vector_of_vector(C, 10);
}

void test_hadamard_production() {
  std::srand(42);
  size_t dim = 32 * 1000 * 1000;
  std::vector<double> A(dim, 0.0); 
  std::vector<double> B(dim, 0.0); 
  std::vector<double> C(dim, 0.0); 
  std::vector<uint64_t> encoded_A(dim, 0);
  std::vector<uint64_t> encoded_B(dim, 0);
  if (party == sci::ALICE) {
    for (size_t i = 0; i < dim; ++i) {
      A[i] = (double) std::rand() / RAND_MAX;
      encoded_A[i] = (int64_t)(A[i] * (1 << SCALER_BIT_LENGTH));
      B[i] = (double) std::rand() / RAND_MAX;
      encoded_B[i] = (int64_t)(B[i] * (1 << SCALER_BIT_LENGTH));
      C[i] = A[i] * B[i];
    }
  }

  FixArray sci_A = fix_op->input(party, dim, (uint64_t*)&(encoded_A[0]), true, 64, SCALER_BIT_LENGTH);
  sci_A = fix_op->reduce(sci_A, 32);
  FixArray sci_B = fix_op->input(party, dim, (uint64_t*)&(encoded_B[0]), true, 64, SCALER_BIT_LENGTH);
  sci_B = fix_op->reduce(sci_B, 32);

  FixArray sci_C = fix_op->mul(sci_A, sci_B, 32);
  sci_C = fix_op->extend(sci_C, 64);

  // fix_op->print(sci_C);

  // printf("---------\n");

  // print_vector(C);
}

int main(int argc, char **argv) {
  ArgMapping amap;

  int int_op = static_cast<int>(op);
  int int_cmp_op = static_cast<int>(cmp_op);
  amap.arg("r", party, "Role of party: sci::ALICE/SERVER = 1; sci::BOB/CLIENT = 2");
  amap.arg("p", port, "Port Number");
  amap.arg("ip", address, "IP Address of server (sci::ALICE)");
  // amap.arg("o", int_op, "FixPoint Primitve Operation");
  amap.arg("c", int_cmp_op, "FixPoint Comparison Operation");
  amap.arg("v", verbose, "Print test inputs/outputs?");
  amap.parse(argc, argv);
  // op = static_cast<Op>(int_op);
  // cmp_op = static_cast<CmpOp>(int_cmp_op);

  //  for (int i=0; i<1; ++i) {

  // printf("-> %d\n", i);

  iopack = new sci::NetIO(party == 1 ? nullptr : address.c_str(), port);
  otpack = new sci::OTPack<sci::NetIO>(iopack, party);

  fix_op = new FixOp(party, iopack, otpack);

  time_log("fixed-point");

  uint64_t comm_start = iopack->counter;

  // test_int_to_float();
  // test_op();
  // test_cmp_op();
  test_matrix_multiplication();
  test_hadamard_production();

  time_log("fixed-point");

  uint64_t comm_end = iopack->counter;
  cout << "Total Comm: " << 8 * (comm_end - comm_start)
      << " bits" << endl;

  delete iopack;
  delete otpack;
  delete fix_op;

  //  }
}
