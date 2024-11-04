with open('big.txt', 'w') as file:
    for _ in range(int(2 ** 30 / 28)):
        print('''abc
de
f

vfs
;afmakfep
ad
f''', file=file)
