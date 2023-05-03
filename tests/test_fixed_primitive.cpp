#include "FloatingPoint/fixed-point.h"
#include "utils/performance.h"
#include <random>
#include <limits>

using namespace sci;
using namespace std;

enum class Op { ADD, MUL, DIV, SQRT, CHEAP_ADD, CHEAP_DIV };

enum class CmpOp { LT, GT, GE, LE };

Op op = Op::DIV;
CmpOp cmp_op = CmpOp::LT;
bool verbose = true;
sci::NetIO *iopack = nullptr;
sci::OTPack<sci::NetIO> *otpack = nullptr;
FixOp *fix_op = nullptr;
int sz = 1;
int party = 1;
string address = "127.0.0.1";
int port = 8000;

void test_op() {
  assert(party == sci::ALICE || party == sci::BOB);
  FixArray fa;
  uint64_t *f_1 = new uint64_t[sz];
  uint64_t *f_2 = new uint64_t[sz];
  uint64_t *f = new uint64_t[sz];
  for (int i = 0; i < sz; i++) {
    f_1[i] = ((i+1)) << 16;
    f_2[i] = ((i+1)*(i+1)) << 16;
  }
  // fa = fix_op->input(party, sz, f_1, false, 64);
  FixArray fa_1 = fix_op->input(sci::ALICE, sz, f_1, true, 32, 16);
  FixArray fa_2 = fix_op->input(sci::BOB, sz, f_2, false, 32, 16);
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
        f[i] = f_1[i] / f_2[i];
        }
        fa = fix_op->div(fa_1, fa_2, 32, 16);
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
           << f_[i] << "\t" << fa_i << endl;
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
  cmp_op = static_cast<CmpOp>(int_cmp_op);

  for (int i=0; i<1000; ++i) {

    printf("-> %d\n", i);

    iopack = new sci::NetIO(party == 1 ? nullptr : address.c_str(), port);
    otpack = new sci::OTPack<sci::NetIO>(iopack, party);

    fix_op = new FixOp(party, iopack, otpack);

    time_log("fixed-point");

    uint64_t comm_start = iopack->counter;

    // test_int_to_float();
    // test_op();
    test_cmp_op();

    time_log("fixed-point");

    uint64_t comm_end = iopack->counter;
    cout << "Total Comm: " << 8 * (comm_end - comm_start)
        << " bits" << endl;

    delete iopack;
    delete otpack;
    delete fix_op;
  }
}
