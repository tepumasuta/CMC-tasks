from typing import Iterable
import copy


class Matrix[T]:
    def __init__(self, raw_matrix: Iterable[Iterable[T]]):
        self.__raw: list[list[T]] = list(map(list, raw_matrix))
        if not self.__raw:
            assert False, "TODO: raise ValueError"
        self.__rows = len(self.__raw)
        self.__cols = len(self.__raw[0])
        if not all(len(row) == self.__cols for row in self.__raw):
            assert False, "TODO: raise ValueError"

    def copy(self) -> "Matrix":
        return Matrix(copy.deepcopy(self.__raw))

    @property
    def rows(self) -> int:
        return self.__rows

    @property
    def cols(self) -> int:
        return self.__cols


def test_matmul():
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[5, 6], [7, 8]]) == Matrix([[19, 43], [22, 50]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[1, 0], [0, 1]]) == Matrix([[1, 2], [3, 4]])
    assert Matrix([[1, 0], [0, 1]]) * Matrix([[1, 2], [3, 4]]) == Matrix([[1, 2], [3, 4]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[1], [0]]) == Matrix([[1], [3]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[0], [1]]) == Matrix([[2], [4]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) * Matrix([[-1], [0], [1]]) == Matrix([[2], [2]])
