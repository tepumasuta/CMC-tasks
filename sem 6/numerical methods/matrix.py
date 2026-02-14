class Matrix: ...


def test_matmul():
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[5, 6], [7, 8]]) == Matrix([[19, 43], [22, 50]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[1, 0], [0, 1]]) == Matrix([[1, 2], [3, 4]])
    assert Matrix([[1, 0], [0, 1]]) * Matrix([[1, 2], [3, 4]]) == Matrix([[1, 2], [3, 4]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[1], [0]]) == Matrix([[1], [3]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[0], [1]]) == Matrix([[2], [4]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) * Matrix([[-1], [0], [1]]) == Matrix([[2], [2]])
