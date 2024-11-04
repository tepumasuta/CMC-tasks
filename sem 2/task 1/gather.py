with open('sort.pas') as file:
    data = filter(lambda line: line.startswith('function') or line.startswith('procedure'), file.read().rstrip().splitlines())
    print('\n'.join(data))
