def main():
    s = ""
    for i in range(100000):
        s += "x"
    return len(s)

if __name__ == "__main__":
    main()
