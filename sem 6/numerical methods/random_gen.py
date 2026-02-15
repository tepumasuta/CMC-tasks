from matrix import Matrix

import random
import itertools


def generate_random_matrix(
    *, size=None, rows=None, cols=None, low=None, high=None, span: None | range = None, value_type=None
) -> Matrix:
    if size is not None and (rows is not None or cols is not None):
        assert False, "TODO: raise ValueError"
    if size is None and (rows is None or cols is None):
        assert False, "TODO: raise ValueError"
    if span is not None and (low is not None or high is not None or value_type is not None):
        assert False, "TODO: raise ValueError"
    if value_type is None:
        t = type(low)
        if t is type(None) or type(high) is float:
            t = type(high)
        value_type = t

    # TODO: Make better check
    # if low is not None and not issubclass(value_type, type(low)):
    #     assert False, "TODO: raise ValueError"
    # if high is not None and not issubclass(value_type, type(high)):
    #     assert False, "TODO: raise ValueError"

    if size is not None:
        cols = size
        rows = size

    if rows <= 0 or cols <= 0:
        assert False, "TODO: raise ValueError"

    if span is not None:
        return Matrix((random.choices(span, k=cols) for _ in range(rows)))
    if value_type is float:
        if low is None:
            low = 0.0
        if high is None:
            high = 1.0
        if low > high:
            assert False, "TODO: raise ValueError"
        return Matrix((random.random() * (high - low) + low for _ in range(cols)) for _ in range(rows))
    if value_type is int:
        if high is None:
            high = 2**64
        if low is None:
            low = -(2**64)
        if low > high:
            assert False, "TODO: raise ValueError"
        return Matrix((random.randint(low, high) for _ in range(cols)) for _ in range(rows))

    assert False, "TODO: raise ValueError"


def generate_random_upper_triangular_matrix(
    *, size=None, low=None, high=None, span: None | range = None, value_type=None
) -> Matrix:
    if span is not None and (low is not None or high is not None or value_type is not None):
        assert False, "TODO: raise ValueError"
    if value_type is None:
        t = type(low)
        if t is type(None) or type(high) is float:
            t = type(high)
        value_type = t

    # TODO: Make better check
    # if low is not None and not issubclass(value_type, type(low)):
    #     assert False, "TODO: raise ValueError"
    # if high is not None and not issubclass(value_type, type(high)):
    #     assert False, "TODO: raise ValueError"

    cols = size
    rows = size

    if rows <= 0 or cols <= 0:
        assert False, "TODO: raise ValueError"

    if span is not None:
        linearized = iter(random.choices(span, k=size * (size + 1) // 2))
    elif value_type is float:
        if low is None:
            low = 0.0
        if high is None:
            high = 1.0
        if low > high:
            assert False, "TODO: raise ValueError"
        linearized = (random.random() * (high - low) + low for i in range(rows) for _ in range(cols - i))
    elif value_type is int:
        if high is None:
            high = 2**64
        if low is None:
            low = -(2**64)
        if low > high:
            assert False, "TODO: raise ValueError"
        linearized = (random.randint(low, high) for i in range(rows) for _ in range(cols - i))
    else:
        assert False, "TODO: raise ValueError"
    return Matrix((next(linearized) if j >= i else 0 for j in range(cols)) for i in range(rows))


def generate_random_symmetric_matrix(
    *, size=None, low=None, high=None, span: None | range = None, value_type=None
) -> Matrix:
    m = generate_random_upper_triangular_matrix(size=size, low=low, high=high, span=span, value_type=value_type)
    for i in range(1, m.rows):
        for j in range(i):
            m[i, j] = m[j, i]
    return m


def test_random_matrices():
    seeds = random.choices(range(2_781_443_143_657_574), k=10**5)
    state = random.getstate()

    seeds_iter = iter(seeds)

    for _, seed in zip(range(1000), seeds_iter):
        random.seed(seed)
        for size in range(1, 6):
            mat = generate_random_matrix(size=size, low=-1000, high=1000)
            assert all(-1000 <= mat[i, j] <= 1000 for i, j in itertools.product(range(size), repeat=2))
            assert mat.cols == mat.rows == size
            assert mat.generic_type == int
        for rows, cols in itertools.product(range(1, 7), repeat=2):
            mat = generate_random_matrix(rows=rows, cols=cols, low=-1000, high=1000)
            assert all(-1000 <= mat[i, j] <= 1000 for i, j in itertools.product(range(rows), range(cols)))
            assert mat.cols == cols and mat.rows == rows
            assert mat.generic_type == int
        for rows, cols in itertools.product(range(1, 4), repeat=2):
            low, high = random.randint(-(10**5), 10**5), random.randint(-(10**5), 10**5)
            low, high = min(low, high), max(low, high)
            mat = generate_random_matrix(rows=rows, cols=cols, low=low, high=high)
            assert all(low <= mat[i, j] <= high for i, j in itertools.product(range(rows), range(cols)))
            assert mat.cols == cols and mat.rows == rows
            assert mat.generic_type == int
        for rows, cols in itertools.product(range(1, 4), repeat=2):
            mat = generate_random_matrix(rows=rows, cols=cols, low=-666, high=15.0)
            assert all(-666 <= mat[i, j] <= 15.0 for i, j in itertools.product(range(rows), range(cols)))
            assert mat.cols == cols and mat.rows == rows
            assert mat.generic_type == float
        for rows, cols in itertools.product(range(1, 4), repeat=2):
            mat = generate_random_matrix(rows=rows, cols=cols, value_type=float)
            assert all(0.0 <= mat[i, j] <= 1.0 for i, j in itertools.product(range(rows), range(cols)))
            assert mat.cols == cols and mat.rows == rows
            assert mat.generic_type == float
        for rows, cols in itertools.product(range(1, 4), repeat=2):
            mat = generate_random_matrix(rows=rows, cols=cols, span=range(-100, 200, 5))
            assert all(
                -100 <= mat[i, j] <= 200 and mat[i, j] % 5 == 0 for i, j in itertools.product(range(rows), range(cols))
            )
            assert mat.cols == cols and mat.rows == rows
            assert mat.generic_type == int

    random.setstate(state)


def test_random_symmetric_matrices():
    seeds = random.choices(range(2_781_443_143_657_574), k=10**5)
    state = random.getstate()

    seeds_iter = iter(seeds)

    for _, seed in zip(range(1000), seeds_iter):
        random.seed(seed)
        for size in range(1, 6):
            mat = generate_random_symmetric_matrix(size=size, low=-1000, high=1000)
            assert all(-1000 <= mat[i, j] <= 1000 for i, j in itertools.product(range(size), repeat=2))
            assert mat.cols == mat.rows == size
            assert mat.generic_type == int
            assert mat.is_symmetric()
        for size in range(1, 6):
            low, high = random.randint(-(10**5), 10**5), random.randint(-(10**5), 10**5)
            low, high = min(low, high), max(low, high)
            mat = generate_random_symmetric_matrix(size=size, low=low, high=high)
            assert all(low <= mat[i, j] <= high for i, j in itertools.product(range(size), repeat=2))
            assert mat.cols == mat.rows == size
            assert mat.generic_type == int
            assert mat.is_symmetric()
        for size in range(1, 6):
            mat = generate_random_symmetric_matrix(size=size, low=-666, high=15.0)
            assert all(-666 <= mat[i, j] <= 15.0 for i, j in itertools.product(range(size), repeat=2))
            assert mat.cols == mat.rows == size
            assert mat.generic_type == float
            assert mat.is_symmetric()
        for size in range(1, 6):
            mat = generate_random_symmetric_matrix(size=size, value_type=float)
            assert all(0.0 <= mat[i, j] <= 1.0 for i, j in itertools.product(range(size), repeat=2))
            assert mat.cols == mat.rows == size
            assert mat.generic_type == float
            assert mat.is_symmetric()
        for size in range(1, 6):
            mat = generate_random_symmetric_matrix(size=size, span=range(-100, 200, 5))
            assert all(
                -100 <= mat[i, j] <= 200 and mat[i, j] % 5 == 0 for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == int
            assert mat.is_symmetric()

    random.setstate(state)


def test_random_upper_triangular_matrix():
    seeds = random.choices(range(2_781_443_143_657_574), k=10**5)
    state = random.getstate()

    seeds_iter = iter(seeds)

    for _, seed in zip(range(1000), seeds_iter):
        random.seed(seed)
        for size in range(1, 6):
            mat = generate_random_upper_triangular_matrix(size=size, low=-1000, high=1000)
            assert all(
                -1000 <= mat[i, j] <= 1000 if j >= i else mat[i, j] == 0
                for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == int
            assert mat.is_upper_triangular()
        for size in range(1, 6):
            low, high = random.randint(-(10**5), 10**5), random.randint(-(10**5), 10**5)
            low, high = min(low, high), max(low, high)
            mat = generate_random_upper_triangular_matrix(size=size, low=low, high=high)
            assert all(
                low <= mat[i, j] <= high if j >= i else mat[i, j] == 0
                for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == int
            assert mat.is_upper_triangular()
        for size in range(1, 6):
            mat = generate_random_upper_triangular_matrix(size=size, low=-666, high=15.0)
            assert all(
                -666 <= mat[i, j] <= 15.0 if j >= i else mat[i, j] == 0
                for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == float
            assert mat.is_upper_triangular()
        for size in range(1, 6):
            mat = generate_random_upper_triangular_matrix(size=size, value_type=float)
            assert all(
                0.0 <= mat[i, j] <= 1.0 if j >= i else mat[i, j] == 0
                for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == float
            assert mat.is_upper_triangular()
        for size in range(1, 6):
            mat = generate_random_upper_triangular_matrix(size=size, span=range(-100, 200, 5))
            assert all(
                -100 <= mat[i, j] <= 200 and mat[i, j] % 5 == 0 if j >= i else mat[i, j] == 0
                for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == int
            assert mat.is_upper_triangular()

    random.setstate(state)

def test_random_lower_triangular_matrix():
    seeds = random.choices(range(2_781_443_143_657_574), k=10**5)
    state = random.getstate()

    seeds_iter = iter(seeds)

    for _, seed in zip(range(1000), seeds_iter):
        random.seed(seed)
        for size in range(1, 6):
            mat = generate_random_lower_triangular_matrix(size=size, low=-1000, high=1000)
            assert all(
                -1000 <= mat[i, j] <= 1000 if j >= i else mat[i, j] == 0
                for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == int
            assert mat.is_lower_triangular()
        for size in range(1, 6):
            low, high = random.randint(-(10**5), 10**5), random.randint(-(10**5), 10**5)
            low, high = min(low, high), max(low, high)
            mat = generate_random_lower_triangular_matrix(size=size, low=low, high=high)
            assert all(
                low <= mat[i, j] <= high if j >= i else mat[i, j] == 0
                for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == int
            assert mat.is_lower_triangular()
        for size in range(1, 6):
            mat = generate_random_lower_triangular_matrix(size=size, low=-666, high=15.0)
            assert all(
                -666 <= mat[i, j] <= 15.0 if j >= i else mat[i, j] == 0
                for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == float
            assert mat.is_lower_triangular()
        for size in range(1, 6):
            mat = generate_random_lower_triangular_matrix(size=size, value_type=float)
            assert all(
                0.0 <= mat[i, j] <= 1.0 if j >= i else mat[i, j] == 0
                for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == float
            assert mat.is_lower_triangular()
        for size in range(1, 6):
            mat = generate_random_lower_triangular_matrix(size=size, span=range(-100, 200, 5))
            assert all(
                -100 <= mat[i, j] <= 200 and mat[i, j] % 5 == 0 if j >= i else mat[i, j] == 0
                for i, j in itertools.product(range(size), repeat=2)
            )
            assert mat.cols == mat.rows == size
            assert mat.generic_type == int
            assert mat.is_lower_triangular()

    random.setstate(state)
