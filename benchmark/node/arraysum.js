function main() {
    const n = 5000000;
    let a = Array(n);
    for (let i = 0; i < n; ++i) a[i] = i;
    let s = 0;
    for (let i = 0; i < n; ++i) s += a[i];
    return s;
}
main();
