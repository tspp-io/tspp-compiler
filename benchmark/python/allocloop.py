def main():
    n = 100000
    sum_ = 0
    for i in range(n):
        arr = [0] * 10
        arr[0] = i
        sum_ += arr[0]
    return sum_

if __name__ == "__main__":
    main()
