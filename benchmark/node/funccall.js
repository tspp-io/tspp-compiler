function inc(v) { return v + 1; }
function main() {
    let x = 0;
    for (let i = 0; i < 5000000; ++i) {
        x = inc(x);
    }
    return x;
}
main();
