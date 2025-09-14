from collections import Counter

def check(space_separated_string: str, filename: str):
    with open(filename, 'w') as file:
        for word, count in sorted(Counter(map(str.lower, space_separated_string.split())).most_common(),
                                  key=lambda p: p[0]):
            print(f'{word} {count}', file=file)
