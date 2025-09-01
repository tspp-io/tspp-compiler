function main() {
    const n = 100000;
    let sum = 0;
    for (let i = 0; i < n; ++i) {
        let arr = Array(10).fill(0);
        arr[0] = i;
        sum += arr[0];
    }
    return sum;
}
main();
