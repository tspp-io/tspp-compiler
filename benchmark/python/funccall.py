def inc(v):
    return v + 1

def main():
    x = 0
    for i in range(5000000):
        x = inc(x)
    return x

if __name__ == "__main__":
    main()
