def find_modified_max_argmax(L, f):
    L = tuple(f(v) for v in L if type(v) == int)
    return L and (max(L), L.index(max(L)))
