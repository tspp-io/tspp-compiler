# tspp — Stabilization Plan + Claude Code Prompts

> Drop this file into the repo as `docs/STABILIZATION_PLAN.md`. Every prompt below tells Claude Code to read it, so it always has the full picture.
> Source of truth for bugs and IDs (C1, H3, …): `docs/KNOWLEDGE_TRANSFER.md` §9–10.

---

## The rule of this plan

**No new language features until Phase 7 is done.** Right now the compiler can silently produce wrong output (strings get overwritten, globals never initialize, errors get swallowed). Adding `#heap` or smart pointers on top of that means building on sand. First make the compiler *honest*: tested, safe, and loud when it fails. Then build features.

Each phase = one branch = one PR. Never two phases in one session.

| Phase | What | Fixes | Effort | Branch |
|---|---|---|---|---|
| 0 | `CLAUDE.md` + identity decision | — | 1 hr | `chore/claude-md` |
| 1 | Restore tests + golden-output harness | §8 | M | `test/harness` |
| 2 | Fix CI so it actually gates | C4, M7 | S | `ci/fix` |
| 3 | Safe driver (no shell injection) | C3, C3b, L4 | M | `fix/safe-driver` |
| 4 | Runtime string ownership | C1, C1b | S–M | `fix/string-ownership` |
| 5 | Run global initializers | C2, C2b, C2c | S–M | `fix/global-ctors` |
| 6 | Fail loudly: diagnostics + IR verify | H1, H6 | M | `fix/diagnostics` |
| 7 | Quick wins: precedence, `boolean`, REPL | H3, H4, H2 | S | `fix/quick-wins` |
| 8 | Docs truth pass | H7a–g | M | `docs/truth-pass` |
| 9 | CMake build types + sanitizer options | M1, M2 | S | `build/cmake-options` |

**After Phase 9 (feature work, real roadmap):** typed arrays + bounds checks (H5) → split `LLVMCodeGenerator.cpp` (M5) → real `#heap` → `#unique<T>` → `#shared<T>`/`#weak<T>` → `@unsafe` pointer semantics. These are what make tspp *tspp*; everything before is groundwork.

---

## Phase 0 — Decide the identity (you, not Claude)

Three descriptions conflict right now:
- Project description: "C++ with TS syntax, interpreted"
- README: "TS syntax + Go semantics, compiled via LLVM"
- `.github/copilot-instructions.md`: "a C++ interpreter"

The code is a **compiler to LLVM**. Recommendation: commit to *"TypeScript syntax with C++-level memory control, compiled to native via LLVM. Safe by default, unsafe when you ask for it."* Drop Go concurrency from the pitch (park it in a "Future ideas" section). Phases 1–7 and 9 don't depend on this; Phase 8 does.

### Prompt 0 — Create CLAUDE.md

```text
Read these files fully before doing anything else:
- docs/KNOWLEDGE_TRANSFER.md
- docs/STABILIZATION_PLAN.md
- docs/MISSING_FEATURES_REPORT.md
- README.md, CMakeLists.txt, src/CMakeLists.txt, build.sh, test.sh

Then create a CLAUDE.md at the repo root (keep it under ~120 lines). It must contain:

1. What tspp is (one paragraph): a compiler from TS-like syntax to LLVM IR, C++17, LLVM 17, Boehm GC, runtime in libcore.a. Not an interpreter. The REPL is not a working JIT.
2. Pipeline and where each stage lives (lexer → parser/AST builders → SemanticAnalyzerVisitor → LLVMCodeGenerator → driver in src/main.cpp → runtime in src/core/syscall/).
3. Exact build and test commands (verify them; don't guess).
4. Working rules for any Claude session in this repo:
   - Read the relevant files before editing. Diagnose before fixing: state the root cause with file:line evidence first.
   - One phase of docs/STABILIZATION_PLAN.md per session/branch. Do not fix unrelated issues you notice; list them at the end instead.
   - Never edit a test's expected output to make it pass unless the expected output is provably wrong; say so explicitly if you do.
   - LLVMCodeGenerator.cpp is ~4.9k lines: use grep to find the function, read only the relevant range, keep diffs minimal.
   - Every fix needs a test in tests/ that fails before and passes after.
   - Run the full test suite before declaring done.
5. Known traps: types are inferred by string-matching names in codegen; test.sh patches IR (void main wrapper) that the driver may not; build forces Debug + ASan.

Do not change any other file. Show me the CLAUDE.md, then stop.
```

**Verify yourself:** CLAUDE.md commands actually work when you run them. Commit.

---

## Phase 1 — Restore tests + golden-output harness

### Prompt 1

```text
Read CLAUDE.md, docs/STABILIZATION_PLAN.md (Phase 1), and docs/KNOWLEDGE_TRANSFER.md §8 (Testing) first.

Goal: a test suite that tells the truth about the compiler. Work on branch test/harness.

Step 1 — Inventory (stop after this and show me):
- List every file under tests/ and examples/ at commit 761708e^ (git ls-tree -r --name-only 761708e^ -- tests examples).
- For each, read it and classify: (a) end-to-end with "// Expected:" comments, (b) end-to-end without expectations, (c) parse-only, (d) needs network/graphics (sys_* net tests, local_tcp_echo.py), (e) benchmark, (f) obsolete syntax (e.g. func/println).
- Build the compiler with ./build.sh and confirm it builds. Report the exact result.
Show me the table and wait for my OK.

Step 2 — Restore and run:
- Restore categories a, b, c into tests/ preserving the folder structure (git checkout 761708e^ -- <paths>). Put benchmarks in tests/bench/ (not run by default). Skip d and f; list them in tests/README.md as "not restored, and why".
- Run each restored test once through the current compiler and record: compiles? links? runs? output?

Step 3 — Harness:
- Write tests/run.sh (bash, set -euo pipefail) that for each tests/**/*.tspp (excluding tests/bench):
  - runs the compiler from a temp working dir so artifacts don't collide,
  - compares stdout to a sibling <name>.expected file,
  - supports <name>.xfail (known-failing; expected to fail, reports XFAIL; an XFAIL that passes is reported as XPASS and is a failure, so we notice fixes),
  - supports parse-only tests via a "// test: parse-only" header (success = compiler exits 0 through codegen),
  - prints a summary line PASS/FAIL/XFAIL/XPASS counts and exits non-zero on any FAIL or XPASS.
- Generate .expected from "// Expected:" comments where they exist. Where they don't, do NOT blindly snapshot current output: show me the output and tell me whether it looks correct; I'll confirm.
- Add these NEW regression tests from KNOWLEDGE_TRANSFER.md §8.3, each marked .xfail with a one-line reason referencing the bug ID:
  - strings survive later string ops (C1): let a = "x" + 1; let b = "y" + 2; console.log(a); → x1
  - non-constant global initializer runs before main (C2)
  - precedence: a & b == c parses as a & (b == c) (H3)
  - boolean variable round-trips true/false (H4)
  - negative tests: undeclared identifier, wrong arity → non-zero exit (H1)
- Hook tests/run.sh into CTest (add_test) so `ctest` works too.

Done when: tests/run.sh exits 0 on a clean build with every known bug showing as XFAIL. Show me the summary output and the diff stat. Do not fix any compiler bugs in this phase.
```

**Verify yourself:** run `tests/run.sh`, then break one `.expected` by hand. It must go red.

---

## Phase 2 — Make CI gate

### Prompt 2

```text
Read CLAUDE.md, docs/STABILIZATION_PLAN.md (Phase 2), docs/KNOWLEDGE_TRANSFER.md §7.1 and §9.4, and .github/workflows/ci.yml first.

First, diagnose: list every reason the current CI cannot gate a PR, with line references. I already know of: triggers on main/develop but the default branch is master; ubuntu-20.04 runner is retired; libcurl not installed (M7); test steps swallow failures with `|| echo`; smoke test uses invalid syntax; build.sh/CI call --version which doesn't exist (L4). Confirm each and find anything else. Stop and show me.

Then fix, on branch ci/fix:
- Trigger on push/PR to master.
- One supported Ubuntu runner (ubuntu-24.04 or 22.04); matrix gcc + clang only if both actually build. Prefer fewer working jobs over many broken ones.
- Install LLVM 17, clang 17, libgc-dev, libcurl4-openssl-dev. Use the signed-by keyring method instead of apt-key.
- Steps: build → tests/run.sh. Any failure fails the job. No `|| echo`, no `continue-on-error`.
- Remove the --version call for now (Phase 3 adds the flag) and note it in the PR description.
- Add libcurl to scripts/install_deps_*.sh, README prerequisites, and CONTRIBUTING.

Done when: I can push the branch and see a green run. Then show me a deliberately broken commit (break one .expected) going red, and revert it.
```

**Verify yourself:** see the red run with your own eyes on GitHub before merging.

---

## Phase 3 — Safe driver (the security fix)

This is your AppSec portfolio piece. Write the PR description like a mini security advisory.

### Prompt 3

```text
Read CLAUDE.md, docs/STABILIZATION_PLAN.md (Phase 3), docs/KNOWLEDGE_TRANSFER.md §9.3 and §10.2 item 1, src/main.cpp, test.sh, and src/CMakeLists.txt first.

Context: src/main.cpp concatenates the input filename into std::system() strings (sed, llvm-as, clang, ./temp_exec). A filename like `x.tspp; touch pwned` executes arbitrary commands. Exit codes are ignored, artifacts go to fixed names in the CWD (temp.bc, temp_exec), libcore.a is found via ./build/src/ so it only works from the repo root, and test.sh does IR patching (void main wrapper, adding a missing main) that the driver may not.

Step 1 — Prove it (stop after this):
- Write a failing test tests/driver/shell_injection.sh: create a source file whose name contains `;` and a harmless payload (touch a marker file in a temp dir). Show the marker gets created today.
- Check whether the driver output differs from test.sh on the same input (i.e. does the driver need the void-main wrapper test.sh applies?). Report with evidence.
Wait for my OK.

Step 2 — Fix on branch fix/safe-driver:
- Replace every std::system call with llvm::sys::ExecuteAndWait using an argv vector (no shell). Find clang/llvm-as with llvm::sys::findProgramByName; allow override via TSPP_CLANG env var.
- Drop the sed step: stop emitting the target triple in codegen, or set the module triple correctly, so no post-processing is needed. Same for the void-main wrapper: move it into codegen if Step 1 showed it's needed.
- Check every child process's exit code; on failure print which stage failed and exit non-zero.
- Write intermediates into a unique temp dir (llvm::sys::fs::createUniqueDirectory) and clean it up unless --keep-temps is passed.
- Locate libcore.a relative to the tspp executable (llvm::sys::fs::getMainExecutable), with TSPP_RUNTIME_LIB as an override. Works from any directory.
- Add CLI: --version, -o <output> (build without running), --emit-llvm (write .ll and stop), --keep-temps, --help. Default behavior (compile and run) stays the same.
- Remove -no-pie. Make -fsanitize=address opt-in via --asan.
- Apply the same changes to the REPL path, or, if it's simpler, route the REPL through the same compile function.
- Update test.sh to be a thin wrapper around the driver (or delete it and update docs/CLAUDE.md), so there is one compile path.

Done when: the injection test passes (marker not created), tests/run.sh is fully green with no new XFAILs, tspp works when run from /tmp, and a forced link failure returns non-zero. Show me the diff of src/main.cpp in full.
```

**Verify yourself:** run `./build/src/tspp 'evil;touch /tmp/pwned.tspp'` style test manually; `cd /tmp && /path/to/tspp file.tspp` works.

---

## Phase 4 — Runtime string ownership

### Prompt 4

```text
Read CLAUDE.md, docs/STABILIZATION_PLAN.md (Phase 4), docs/KNOWLEDGE_TRANSFER.md §9.1 (C1, C1b) and §6.4 (runtime ABI) first. Then read src/core/syscall/syscall_wrapper.cpp fully.

Diagnose first (stop after this): list every runtime function that returns a pointer into a shared static buffer or a function-local static initialized once (tspp_string_concat, tspp_int_to_string, the other *_to_string helpers, tspp_sys_read_file, tspp_sys_get_cwd, and any others you find). For each: file:line, what goes wrong, and which test reproduces it. Also confirm how Boehm GC is initialized (is GC_INIT called anywhere?).

Fix on branch fix/string-ownership:
- Every function returning a string to tspp code returns a fresh, GC-owned buffer (GC_MALLOC_ATOMIC + copy, NUL-terminated). No shared statics.
- Ensure GC_INIT runs once before user code (e.g. from the generated main or a runtime init function the codegen calls first).
- tspp_sys_read_file and tspp_sys_get_cwd compute their result on every call.
- Check lib/system.tspp's printf buffer path still works.
- Remove the .xfail from the C1 regression test. Add tests: string built in a loop of 1000 concats keeps correct content; read_file on two different files returns two different contents; get_cwd changes after set_cwd.

Done when: tests/run.sh green, C1 test is a real PASS. Show me the diff of syscall_wrapper.cpp.
```

---

## Phase 5 — Run global initializers

### Prompt 5

```text
Read CLAUDE.md, docs/STABILIZATION_PLAN.md (Phase 5), docs/KNOWLEDGE_TRANSFER.md §9.1 (C2, C2b, C2c) and §4.2 (the "global variable with a non-constant initializer" walkthrough) first.

In src/codegen/LLVMCodeGenerator.cpp, use grep to locate visit(ProgramNode&), visit(VarDecl&), and every reference to __tspp_global_ctors. Read only those ranges.

Diagnose first (stop after this): explain with line numbers (1) why __tspp_global_ctors is never executed, (2) where global initializer expressions are emitted at a stale insertion point (C2b), (3) what happens to top-level non-declaration statements (C2c). Propose the fix in plain English before writing code.

Fix on branch fix/global-ctors:
- Register __tspp_global_ctors in llvm.global_ctors (priority 65535), OR call it explicitly at the start of the generated main. Pick one, explain why, and make sure it runs after the runtime/GC init from Phase 4.
- Evaluate global initializers only inside __tspp_global_ctors' insertion point; save/restore the builder's insertion point around it.
- Top-level statements: decide (with me) between "compile error: statements not allowed at top level" and "run them in order inside the ctor". Ask me before implementing.
- Remove .xfail from the C2 test. Add tests: global initialized from a function call; global depending on another global; initialization order across an import.

Done when: tests/run.sh green, and llvm-as accepts every generated .ll in the suite.
```

---

## Phase 6 — Fail loudly

### Prompt 6

```text
Read CLAUDE.md, docs/STABILIZATION_PLAN.md (Phase 6), docs/KNOWLEDGE_TRANSFER.md §9.2 (H1, H6) and §6.5 first. Then read src/core/diagnostics/*, src/main.cpp, and grep for ErrorReporter, isStatementStart, and the semantic error vector.

Goal: any invalid program produces a non-zero exit and a message with file:line:col. No silent skipping.

Diagnose first (stop after this): map every place errors are currently dropped: lexer errors ignored in main.cpp, parser silently skipping unknown tokens, isStatementStart missing identifiers, enum/namespace dropped token-by-token, semantic errors without location, generate() patching unterminated blocks. File:line for each.

Fix on branch fix/diagnostics, in this order, running tests/run.sh after each:
1. Wire the existing ErrorReporter (or replace it if it's unusable; justify) as the single diagnostics sink with file:line:col and a severity.
2. Lexer errors → reported, compilation stops.
3. Parser: unknown/unexpected tokens → error with location, then synchronize to the next statement boundary so multiple errors can be reported. enum/namespace → explicit "not supported yet" error, not silent drop.
4. Semantic errors carry the AST node's location.
5. After codegen, run llvm::verifyModule; on failure print the verifier output and exit non-zero (internal compiler error). Remove the silent block-patching in generate(); if that causes existing tests to fail, show me which ones — those are real codegen bugs, mark them .xfail with a note, don't paper over them.
6. Remove .xfail from the H1 negative tests; add negative tests per error class with expected stderr substrings (extend tests/run.sh to support <name>.expected_err).

Done when: tests/run.sh green, and each negative test prints a located error.
```

---

## Phase 7 — Quick wins

### Prompt 7

```text
Read CLAUDE.md, docs/STABILIZATION_PLAN.md (Phase 7), docs/KNOWLEDGE_TRANSFER.md §9.1 (H2, H3, H4) and docs/grammar.md's expression section first.

Three small, separate fixes on branch fix/quick-wins, one commit each, tests after each:

1. Precedence (H3): read ExpressionBuilder.cpp around the precedence table. Make it match standard TS/C precedence (unary > * / % > + - > shifts > relational > equality > & > ^ > | > && > ||), add unary +. Update docs/grammar.md if it disagrees with TS. Remove .xfail from the precedence test and add a table-driven test covering every level.
2. boolean (H4): centralize primitive type names in one place (a single table/enum used by lexer, semantic analyzer, and getLLVMType). Map both boolean and bool to i1 — or pick one and make the other a compile error; ask me which.
3. REPL (H2): the JIT path is unreachable and generateModuleForREPL has no return (UB). Remove the dead JIT code and make REPL mode honestly "compile and run each chunk" through the Phase 3 driver path, with the banner saying so. Remove orphaned interpreter code listed in KNOWLEDGE_TRANSFER.md §3.3 only if nothing references it.

Done when: tests/run.sh green.
```

---

## Phase 8 — Docs truth pass (after the Phase 0 decision)

### Prompt 8

```text
Read CLAUDE.md, docs/STABILIZATION_PLAN.md, docs/KNOWLEDGE_TRANSFER.md §1.2 and §9.6, and every file in docs/, plus README.md, CONTRIBUTING.md, SECURITY.md, .github/copilot-instructions.md.

Positioning (decided): "<PASTE YOUR PHASE 0 DECISION HERE>"

On branch docs/truth-pass:
- README: rewrite around the positioning. Feature list split into "Works today" (only things with a passing test in tests/; link the test) and "Planned". The hello-world must be a program that is in tests/ and passes. Fix clone URL to github.com/tspp-io/tspp-compiler and badge links. Go concurrency moves to "Future ideas" or is removed.
- docs/grammar.md: remove the chatbot residue at the top and bottom; make precedence and type names match the implementation after Phase 7.
- docs/language_spec.md and docs/architecture.md: remove claims of #heap/SIMD/smart pointers/JIT behaviour that don't exist; mark them planned.
- .github/copilot-instructions.md: fix "C++ interpreter" or delete the file.
- CONTRIBUTING.md: real directory tree, real test command.
- SECURITY.md: replace the placeholder with ranahiren27@gmail.com or GitHub private vulnerability reporting (ask me which).
- docs/todo.md + todo.ebnf: delete or fold into a single ROADMAP.md that matches this plan's "After Phase 9" list.
- Update KNOWLEDGE_TRANSFER.md §1.2 status table and §9 to mark fixed items with the PR that fixed them.

Verify every code sample in the docs compiles with the current build (write a small script for it). Show me the README diff first, before touching the other files.
```

---

## Phase 9 — CMake options

### Prompt 9

```text
Read CLAUDE.md, docs/STABILIZATION_PLAN.md (Phase 9), docs/KNOWLEDGE_TRANSFER.md §9.4 (M1, M2), CMakeLists.txt, src/CMakeLists.txt, build.sh and lsan.supp first.

On branch build/cmake-options:
- Stop forcing CMAKE_BUILD_TYPE=Debug; default to Debug only when unset. Add options TSPP_ENABLE_ASAN (default ON for Debug, OFF for Release) and TSPP_ENABLE_UBSAN.
- Narrow lsan.supp to genuine third-party leaks (Boehm GC/LLVM internals) with a comment per entry; remove blanket malloc/new suppressions. If real leaks appear in the compiler, list them; fix only trivial ones, open TODOs for the rest.
- build.sh accepts --release.
- Remove committed compile_commands.json (generate via CMAKE_EXPORT_COMPILE_COMMANDS, gitignore it) and dedupe .vscode/tasks.json.
- Untangle the core ↔ tokens circular library dependency if it's straightforward; otherwise document it and stop.
- CI: add a Release build job running tests/run.sh.

Done when: Debug and Release both build and pass tests locally and in CI.
```

---

## How to run a session (every phase)

1. `git checkout master && git pull && git checkout -b <branch>`
2. Paste the prompt. Let it diagnose. **Read the diagnosis before saying OK.** This is where you catch wrong assumptions.
3. After it says done: run `tests/run.sh` yourself, read the diff, then open the PR.
4. Merge, then update `KNOWLEDGE_TRANSFER.md` status (or let Phase 8 batch it).

If a session drifts ("while I was here I also refactored…"): reject, `git restore`, re-run with "Only Phase N. List other issues, don't fix them."