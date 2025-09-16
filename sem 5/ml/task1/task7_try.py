# v1 = """
def find_modified_max_argmax(L, f):
    L = tuple(f(v) for v in L if type(v) == int)
    return L and (max(L), L.index(max(L)))
# """
# remove_whitespace = lambda s: s.replace(' ','').replace('\n', '')

# print(len(remove_whitespace(v1)))

class A(int):
    def __init__(self):
        pass


print(find_modified_max_argmax([1, 3, 4, 4.5], lambda x: x ** 2))
print(find_modified_max_argmax(["str", "1", 23, 4, 65], lambda x: -x + 2))
print(find_modified_max_argmax(["str", 4, 4.5, 2, '-2', '3', -8], lambda x: abs(x) - 10))
print(find_modified_max_argmax(["str", '-2', '3'], lambda x: abs(x) - 10))
print(find_modified_max_argmax([A(), 1, 2], lambda x: x))
print(find_modified_max_argmax([], lambda x: abs(x) - 10))