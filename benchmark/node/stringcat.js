function main() {
    let s = "";
    for (let i = 0; i < 100000; ++i) {
        s += "x";
    }
    return s.length;
}
main();
