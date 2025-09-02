
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace interpreter {

enum class OpCode { ADD, INC, HALT };

struct Value {
  enum { INT, BOXED } tag;
  union {
    int64_t i;
    void* boxed;
  };
  Value(int64_t v) : tag(INT), i(v) {}
  Value(void* b) : tag(BOXED), boxed(b) {}
};

struct Instruction {
  OpCode op;
  int dst, src1, src2;
};

bool fast_int_enabled = true;  // Set false to disable fast path

// Example: interpret a sequence of instructions
int interpret(const std::vector<Instruction>& code) {
  std::vector<Value> regs(8, Value((int64_t)0));
  for (size_t pc = 0; pc < code.size(); ++pc) {
    const auto& inst = code[pc];
    switch (inst.op) {
      case OpCode::ADD:
        if (fast_int_enabled && regs[inst.src1].tag == Value::INT &&
            regs[inst.src2].tag == Value::INT) {
          regs[inst.dst] = Value(regs[inst.src1].i + regs[inst.src2].i);
        } else {
          // Slow path: box/unbox, type checks, etc.
          // ...existing code...
          regs[inst.dst] = Value((int64_t)42);  // stub
        }
        break;
      case OpCode::INC:
        if (fast_int_enabled && regs[inst.src1].tag == Value::INT) {
          regs[inst.dst] = Value(regs[inst.src1].i + 1);
        } else {
          // Slow path
          regs[inst.dst] = Value((int64_t)43);  // stub
        }
        break;
      case OpCode::HALT:
        return regs[inst.dst].tag == Value::INT ? regs[inst.dst].i : 0;
    }
  }
  return 0;
}

// Example usage: intinc microbenchmark
void run_intinc_bench() {
  std::vector<Instruction> code;
  // x = 0; for i = 0; i < N; i++ { x = x + 1 }
  int N = 30000000;
  code.push_back({OpCode::ADD, 0, 0, 0});  // x = 0
  code.push_back({OpCode::ADD, 1, 1, 1});  // i = 0
  for (int i = 0; i < N; ++i) {
    code.push_back({OpCode::INC, 0, 0, 0});  // x = x + 1
  }
  code.push_back({OpCode::HALT, 0, 0, 0});
  int result = interpret(code);
  std::cout << "intinc result: " << result << std::endl;
}

}  // namespace interpreter
