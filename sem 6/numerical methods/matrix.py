from typing import Iterable
import copy
import itertools


class Matrix[T]:
    def __init__(self, raw_matrix: Iterable[Iterable[T]]):
        self.__raw: list[list[T]] = list(map(list, raw_matrix))
        if not self.__raw:
            assert False, "TODO: raise ValueError"
        self.__rows = len(self.__raw)
        self.__cols = len(self.__raw[0])
        if not all(len(row) == self.__cols for row in self.__raw):
            assert False, "TODO: raise ValueError"

    def copy(self) -> "Matrix[T]":
        return Matrix(copy.deepcopy(self.__raw))

    @property
    def rows(self) -> int:
        return self.__rows

    @property
    def cols(self) -> int:
        return self.__cols

    def __mul__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(
                (sum(self[i, k] * other[k, j] for k in range(self.cols)) for j in range(other.cols))
                for i in range(self.rows)
            )
        if isinstance(other, (int, float)):
            return Matrix([[v * other for v in row] for row in self.__raw])
        return NotImplemented

    def __rmul__(self, other) -> "Matrix":
        if isinstance(other, (int, float)):
            return self.__mul__(other)
        return NotImplemented

    def __getitem__(self, index: tuple[int, int]) -> T:
        i, j = index
        if i not in range(self.rows) or j not in range(self.cols):
            assert False, "TODO: raise KeyError"
        return self.__raw[i][j]

    def __eq__(self, other) -> bool:
        if not isinstance(other, Matrix):
            return False
        return (
            self.cols == other.cols
            and self.rows == other.rows
            and all(self[i, j] == other[i, j] for i, j in itertools.product(range(self.rows), range(self.cols)))
        )

    def __repr__(self) -> str:
        max_spacing = max(len(str(v)) for row in self.__raw for v in row)
        return f'{'\n'.join(f'[{' '.join(str(v).ljust(max_spacing)for v in row)}]' for row in self.__raw)}'


def test_matmul():
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[5, 6], [7, 8]]) == Matrix([[19, 22], [43, 50]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[1, 0], [0, 1]]) == Matrix([[1, 2], [3, 4]])
    assert Matrix([[1, 0], [0, 1]]) * Matrix([[1, 2], [3, 4]]) == Matrix([[1, 2], [3, 4]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[1], [0]]) == Matrix([[1], [3]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[0], [1]]) == Matrix([[2], [4]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) * Matrix([[-1], [0], [1]]) == Matrix([[2], [2]])


def test_scalmul():
    assert Matrix([[1, 2], [3, 4]]) * 2 == Matrix([[2, 4], [6, 8]])
    assert 2 * Matrix([[1, 2], [3, 4]]) == Matrix([[2, 4], [6, 8]])
    assert Matrix([[1, 2], [3, 4]]) * 0.5 == Matrix([[0.5, 1], [1.5, 2]])
    assert 0.5 * Matrix([[1, 2], [3, 4]]) == Matrix([[0.5, 1], [1.5, 2]])


def test_scaldiv():
    assert Matrix([[1, 2], [3, 4]]) / 2 == Matrix([[0.5, 1], [1.5, 2]])
    assert Matrix([[1, 2], [3, 4]]) // 2 == Matrix([[0, 1], [1, 2]])
    assert Matrix([[1, 2], [3, 4]]) / 0.5 == Matrix([[2, 4], [6, 8]])
