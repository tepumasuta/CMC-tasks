find_modified_max_argmax = lambda L, f: (L := tuple(f(v) for v in L if type(v) == int)) and (max(L), L.index(max(L)))
