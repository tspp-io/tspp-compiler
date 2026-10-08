# tspp tests

```bash
./build.sh                 # build the compiler + runtime first
tests/run.sh               # run everything (except tests/bench/)
tests/run.sh regression    # only tests whose path contains "regression"
cd build && ctest --output-on-failure   # same suite through CTest
```

`run.sh` prints one line per test and a summary
`Summary: N PASS, N FAIL, N XFAIL, N XPASS`. It exits non-zero on any FAIL or XPASS.

## How a test is run

Each `tests/**/*.tspp` (except `tests/bench/`) is compiled in place with the real
driver (`build/src/tspp -o <tmp>/exe file.tspp`, or `--emit-llvm` for parse-only),
with a fresh temp directory as the working directory; the executable is then run
there. Compiling and running each have a timeout (`TSPP_TEST_TIMEOUT`, default 20 s).
Only **stdout** is compared, so ASan/LSan noise on stderr doesn't matter.

Codegen wraps a `main(): void` in an `i32 main()` that returns 0, but new tests
should declare `function main(): int`.

## Test kinds

| Kind | How to declare | Passes when |
|---|---|---|
| run (default) | sibling `<name>.expected`; optional `// exit: <n>` | `tspp -o` exits 0, stdout == `.expected`, exit status == `<n>` (default 0) |
| parse-only | `// test: parse-only` in the first 20 lines | `tspp --emit-llvm` exits 0 and the IR passes `llvm-as` (not linked or run) |
| compile-error | `// test: compile-error`, plus optional `// error: <text>` lines | `tspp -o` exits non-zero and each `<text>` appears in stderr |
| driver | `tests/driver/<name>.sh` | the script exits 0 (it gets `TSPP` and a private `TMPDIR`) |

`<name>` is the file name without `.tspp`, e.g. `sample.test.tspp` → `sample.test.expected`.

**Known failures:** a sibling `<name>.xfail` marks the test as expected to fail. The
first line is the reason and names the bug ID from `docs/KNOWLEDGE_TRANSFER.md` §9.
A failing xfail test is reported as `XFAIL`. A passing one is `XPASS` and fails the
run: delete the `.xfail` file in the PR that fixes the bug.

Never edit a `.expected` to make a test pass unless the expected output is provably
wrong, and say so in the PR when you do.

## Where the expectations come from

- Restored end-to-end tests (`final`, `classes_interface.test`,
  `interfaces_statements_expressions.test`, `sample*.test`,
  `functions/default_params.test`): `.expected` was written from the
  `// Expected:` / trailing `// <output>` comments, using the source to resolve
  comment typos (for example, `formatPoint(2.2, 3.3,'Q'): ` is the literal label).
  Floats are expected in shortest form (`3.5`, `(2, 3)`), as the comments say.
  The object-literal lines in `sample*.test.expected` (`i: { name: "John", id: 1 }`)
  are the author's intent and aren't pinned down by any spec yet. Revisit them when
  object literals get real support.
- `sys_test*.expected` (`linux`): checked by hand against the source.
- `basics/` and `regression/` are new tests. Their expected output was derived by hand
  from the source semantics, not snapshotted.

## Layout

| Path | What |
|---|---|
| `*.tspp` (top level), `functions/`, `grammar/`, `loops/`, `modifiers/`, `types/` | Restored from `761708e^` (`git ls-tree -r 761708e^ -- tests`) |
| `examples/system.tspp` | Restored `examples/system.tspp` (same as `lib/system.tspp`), parse-only |
| `basics/` | Passing smoke tests for the parts of `final.tspp` that work today |
| `regression/` | Known-bug pins from KNOWLEDGE_TRANSFER §8.3 (C1, C2, H1, H3, H4) |
| `driver/` | Driver tests: shell injection (C3), run from another directory, failing build stages, CLI flags and exit status |
| `bench/` | Micro-benchmarks. **Not run** by `run.sh`. |

Notes on specific restored tests:

- `grammar/for_forVarDecl.tspp` was "build-only", but the program is invalid. It
  assigns to a `const` loop variable and uses `i` outside its `for` scope. It is
  kept as a compile-error test.
- `console_impl_in_tspp.tspp` (a user-defined `console` class) and `sys_io_env.tspp`
  (prints CWD and `$PATH`) are parse-only. Their runtime output is either undefined
  or depends on the environment.
- `bench/allocloop.tspp` and `bench/arraysum.tspp` use a `memory.alloc` stdlib
  that doesn't exist and fail semantic analysis. `bench/stringcat.tspp` uses
  `string_util.concat` and `.length`.

## Not restored (and why)

| File at `761708e^` | Reason |
|---|---|
| `tests/sys_network.tspp`, `tests/run_network_test.sh`, `tests/local_tcp_echo.py` | Need local HTTP/TCP servers |
| `tests/sys_server.tspp`, `sys_server_concurrent.tspp`, `sys_server_timeout.tspp` | Open TCP ports (9002–9004). Timing-dependent. |
| `tests/sys_graphics.tspp` | Graphics/canvas output (writes a PPM file) |
| `examples/hello_world.tspp`, `functions.tspp`, `variables.tspp` | Obsolete syntax (`func`, `println`, `number`, `.toString()`) |
| `examples/README.md` | Describes the obsolete examples |
