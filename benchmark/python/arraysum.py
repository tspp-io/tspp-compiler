def main():
    n = 5000000
    a = [i for i in range(n)]
    s = 0
    for i in range(n):
        s += a[i]
    return s

if __name__ == "__main__":
    main()
