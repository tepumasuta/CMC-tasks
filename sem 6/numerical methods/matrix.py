from typing import Iterable
import copy
import itertools
import math


class Matrix[U]:
    def __init__(self, raw_matrix: Iterable[Iterable[U]]):
        self.__raw: list[list[U]] = list(map(list, raw_matrix))
        if not self.__raw:
            assert False, "TODO: raise ValueError"
        self.__rows = len(self.__raw)
        self.__cols = len(self.__raw[0])
        if not all(len(row) == self.__cols for row in self.__raw):
            assert False, "TODO: raise ValueError"

    def copy(self) -> "Matrix[U]":
        return Matrix(copy.deepcopy(self.__raw))

    @property
    def rows(self) -> int:
        return self.__rows

    @property
    def cols(self) -> int:
        return self.__cols

    @property
    def T(self) -> "Matrix[T]":
        return Matrix(zip(*self.__raw))

    def __add__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(((self[i, j] + other[i, j] for j in range(self.cols)) for i in range(self.rows)))
        if isinstance(other, (int, float)):
            return Matrix(((self[i, j] + other for j in range(self.cols)) for i in range(self.rows)))
        return NotImplemented

    def __radd__(self, other) -> "Matrix":
        if isinstance(other, (int, float)):
            return self.__add__(other)
        return NotImplemented

    def __iadd__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            if other is self:
                tmp = self.__raw
                self.__raw = [[tmp[i][j] + tmp[i][j] for j in range(self.cols)] for i in range(self.rows)]
            else:
                self.__raw = [[self[i, j] + other[i, j] for j in range(self.cols)] for i in range(self.rows)]
            return self
        if isinstance(other, (int, float)):
            self.__raw = [[self[i, j] + other for j in range(self.cols)] for i in range(self.rows)]
            return self
        return NotImplemented

    def __sub__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(((self[i, j] - other[i, j] for j in range(self.cols)) for i in range(self.rows)))
        if isinstance(other, (int, float)):
            return Matrix(((self[i, j] - other for j in range(self.cols)) for i in range(self.rows)))
        return NotImplemented

    def __isub__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            if other is self:
                tmp = self.__raw
                self.__raw = [[tmp[i][j] - tmp[i][j] for j in range(self.cols)] for i in range(self.rows)]
            else:
                self.__raw = [[self[i, j] - other[i, j] for j in range(self.cols)] for i in range(self.rows)]
            return self
        if isinstance(other, (int, float)):
            self.__raw = [[self[i, j] - other for j in range(self.cols)] for i in range(self.rows)]
            return self
        return NotImplemented

    def __mul__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(((self[i, j] * other[i, j] for j in range(self.cols)) for i in range(self.rows)))
        if isinstance(other, (int, float)):
            return Matrix(((v * other for v in row) for row in self.__raw))
        return NotImplemented

    def __rmul__(self, other) -> "Matrix":
        if isinstance(other, (int, float)):
            return self.__mul__(other)
        return NotImplemented

    def __imul__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            if other is self:
                tmp = self.__raw
                self.__raw = [[tmp[i][j] * tmp[i][j] for j in range(self.cols)] for i in range(self.rows)]
            else:
                self.__raw = [[self[i, j] * other[i, j] for j in range(self.cols)] for i in range(self.rows)]
            return self
        if isinstance(other, (int, float)):
            self.__raw = [[v * other for v in row] for row in self.__raw]
            return self
        return NotImplemented

    def __truediv__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(((self[i, j] / other[i, j] for j in range(self.cols)) for i in range(self.rows)))
        if isinstance(other, (int, float)):
            return Matrix(((v / other for v in row) for row in self.__raw))
        return NotImplemented

    def __itruediv__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            if other is self:
                tmp = self.__raw
                self.__raw = [[tmp[i][j] / tmp[i][j] for j in range(self.cols)] for i in range(self.rows)]
            else:
                self.__raw = [[self[i, j] / other[i, j] for j in range(self.cols)] for i in range(self.rows)]
            return self
        if isinstance(other, (int, float)):
            self.__raw = [[v / other for v in row] for row in self.__raw]
            return self
        return NotImplemented

    def __floordiv__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(((self[i, j] // other[i, j] for j in range(self.cols)) for i in range(self.rows)))
        if isinstance(other, int):
            return Matrix(((v // other for v in row) for row in self.__raw))
        return NotImplemented

    def __ifloordiv__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            if other is self:
                tmp = self.__raw
                self.__raw = [[tmp[i][j] // tmp[i][j] for j in range(self.cols)] for i in range(self.rows)]
            else:
                self.__raw = [[self[i, j] // other[i, j] for j in range(self.cols)] for i in range(self.rows)]
            return self
        if isinstance(other, int):
            self.__raw = [[v // other for v in row] for row in self.__raw]
            return self
        return NotImplemented

    def __matmul__(self, other) -> "Matrix[U]":
        if isinstance(other, Matrix):
            if self.cols != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(
                (sum(self[i, k] * other[k, j] for k in range(self.cols)) for j in range(other.cols))
                for i in range(self.rows)
            )
        return NotImplemented

    def __imatmul__(self, other) -> "Matrix[U]":
        if isinstance(other, Matrix):
            if self.cols != other.rows:
                assert False, "TODO: raise ValueError"
            if other is self:
                tmp = self.__raw
                self.__raw = [
                    [sum(tmp[i][k] * other[k][j] for k in range(self.cols)) for j in range(other.cols)]
                    for i in range(self.rows)
                ]
            else:
                self.__raw = [
                    [sum(self[i, k] * other[k, j] for k in range(self.cols)) for j in range(other.cols)]
                    for i in range(self.rows)
                ]
                self.__cols = other.cols
            return self
        return NotImplemented

    def __getitem__(self, index: tuple[int, int]) -> U:
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
            and (
                all(self[i, j] == other[i, j] for i, j in itertools.product(range(self.rows), range(self.cols)))
                if self.cols == 0
                or not any(
                    isinstance(self[i, j], float) for i, j in itertools.product(range(self.rows), range(self.cols))
                )
                else all(
                    math.isclose(self[i, j], other[i, j])
                    for i, j in itertools.product(range(self.rows), range(self.cols))
                )
            )
        )

    def __repr__(self) -> str:
        max_spacing = max(len(str(v)) for row in self.__raw for v in row)
        return f'{'\n'.join(f'[{' '.join(str(v).ljust(max_spacing)for v in row)}]' for row in self.__raw)}'

    @staticmethod
    def vec(vec: Iterable[U]) -> "Matrix[U]":
        return Matrix([[v] for v in vec])

    @staticmethod
    def zero(rows: int, cols: int) -> "Matrix[U]":
        return Matrix([[0] * cols for _ in range(rows)])

    @staticmethod
    def ones(rows: int, cols: int) -> "Matrix[U]":
        return Matrix([[1] * cols for _ in range(rows)])

    @staticmethod
    def diag(vec: Iterable[U]) -> "Matrix[U]":
        vec = tuple(vec)
        return Matrix((0,) * i + (v,) + (0,) * (len(vec) - i - 1) for i, v in enumerate(vec))

    @staticmethod
    def eye(size: int) -> "Matrix[U]":
        return Matrix.diag((1,) * size)


def test_matrix_constructors():
    assert Matrix([[1], [2], [3]]) == Matrix.vec([1, 2, 3])
    assert Matrix.zero(1, 3) == Matrix([[0, 0, 0]])
    assert Matrix.zero(3, 1) == Matrix.vec([0, 0, 0])
    assert Matrix.zero(2, 2) == Matrix([[0, 0], [0, 0]])
    assert Matrix.ones(1, 3) == Matrix([[1, 1, 1]])
    assert Matrix.ones(3, 1) == Matrix.vec([1, 1, 1])
    assert Matrix.ones(2, 2) == Matrix([[1, 1], [1, 1]])
    assert Matrix.eye(1) == Matrix([[1]])
    assert Matrix.eye(2) == Matrix([[1, 0], [0, 1]])
    assert Matrix.eye(3) == Matrix([[1, 0, 0], [0, 1, 0], [0, 0, 1]])
    assert Matrix.eye(3) == Matrix.diag([1, 1, 1])
    assert Matrix.diag([1, 2, 3]) == Matrix([[1, 0, 0], [0, 2, 0], [0, 0, 3]])
    assert Matrix.diag([42, 69]) == Matrix([[42, 0], [0, 69]])
    assert Matrix.diag([777]) == Matrix([[777]])


def test_matrix_conviniecnes():
    assert Matrix([[1, 2], [3, 4]]).T == Matrix([[1, 3], [2, 4]])
    assert Matrix.vec([1, 2, 3]).T == Matrix([[1, 2, 3]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]).T == Matrix([[1, 4], [2, 5], [3, 6]])


def test_matmul():
    assert Matrix([[1, 2], [3, 4]]) @ Matrix([[5, 6], [7, 8]]) == Matrix([[19, 22], [43, 50]])
    assert Matrix([[1, 2], [3, 4]]) @ Matrix.eye(2) == Matrix([[1, 2], [3, 4]])
    assert Matrix.eye(2) @ Matrix([[1, 2], [3, 4]]) == Matrix([[1, 2], [3, 4]])
    assert Matrix([[1, 2], [3, 4]]) @ Matrix([[1], [0]]) == Matrix([[1], [3]])
    assert Matrix([[1, 2], [3, 4]]) @ Matrix([[0], [1]]) == Matrix([[2], [4]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) @ Matrix([[-1], [0], [1]]) == Matrix([[2], [2]])


def test_scalops():
    assert Matrix([[1, 2], [3, 4]]) * 2 == Matrix([[2, 4], [6, 8]])
    assert 2 * Matrix([[1, 2], [3, 4]]) == Matrix([[2, 4], [6, 8]])
    assert Matrix([[1, 2], [3, 4]]) * 0.5 == Matrix([[0.5, 1], [1.5, 2]])
    assert 0.5 * Matrix([[1, 2], [3, 4]]) == Matrix([[0.5, 1], [1.5, 2]])
    assert Matrix([[1, 2], [3, 4]]) / 2 == Matrix([[0.5, 1], [1.5, 2]])
    assert Matrix([[1, 2], [3, 4]]) // 2 == Matrix([[0, 1], [1, 2]])
    assert Matrix([[1, 2], [3, 4]]) / 0.5 == Matrix([[2, 4], [6, 8]])


def test_matops():
    assert Matrix([[1, 2], [3, 4]]) + Matrix([[5, 6], [7, 8]]) == Matrix([[6, 8], [10, 12]])
    assert Matrix([[1, 2], [3, 4]]) - Matrix.eye(2) == Matrix([[0, 2], [3, 3]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) + Matrix.ones(2, 3) == Matrix([[2, 3, 4], [5, 6, 7]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) - Matrix.ones(2, 3) == Matrix([[0, 1, 2], [3, 4, 5]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[5, 6], [7, 8]]) == Matrix([[5, 12], [21, 32]])
    assert Matrix([[1, 2], [3, 4]]) / Matrix([[2, 2], [2, 2]]) == Matrix([[0.5, 1.0], [1.5, 2.0]])
    assert Matrix([[1, 2], [3, 4]]) // Matrix([[2, 2], [2, 2]]) == Matrix([[0, 1], [1, 2]])


def test_scalmatops():
    assert Matrix([[1, 2], [3, 4]]) + 0 == Matrix([[1, 2], [3, 4]])
    assert 0 + Matrix([[1, 2], [3, 4]]) == Matrix([[1, 2], [3, 4]])
    assert Matrix([[1, 2], [3, 4]]) + 1 == Matrix([[2, 3], [4, 5]])
    assert Matrix([[1, 2], [3, 4]]) - 1 == Matrix([[0, 1], [2, 3]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) + 0.3 == Matrix([[1.3, 2.3, 3.3], [4.3, 5.3, 6.3]])
    assert 0.3 + Matrix([[1, 2, 3], [4, 5, 6]]) == Matrix([[1.3, 2.3, 3.3], [4.3, 5.3, 6.3]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) - 0.3 == Matrix([[0.7, 1.7, 2.7], [3.7, 4.7, 5.7]])


def test_inplaceops():
    vec = Matrix.vec([1, 2, 3])
    vec += vec
    assert vec == Matrix.vec([2, 4, 6])
    vec //= 2
    assert vec == Matrix.vec([1, 2, 3])
    vec *= 3
    assert vec == Matrix.vec([3, 6, 9])
    vec /= 1.5
    assert vec == Matrix.vec([2, 4, 6])
    vec -= vec
    assert vec == Matrix.zero(3, 1)
    vec += 1
    assert vec == Matrix.vec([1, 1, 1])
    vec *= Matrix.vec([1, 2, 3])
    assert vec == Matrix.vec([1, 2, 3])
    vec //= vec
    assert vec == Matrix.vec([1, 1, 1])
    vec /= Matrix.vec([1, 2, 0.5])
    assert vec == Matrix.vec([1, 0.5, 2])
    vec += 2.5
    assert vec == Matrix.vec([3.5, 3.0, 4.5])
    vec -= 2
    assert vec == Matrix.vec([1.5, 1.0, 2.5])
    vec -= 1.5
    assert vec == Matrix.vec([0.0, -0.5, 1.0])
    vec *= 1.5
    assert vec == Matrix.vec([0.0, -0.75, 1.5])
    vec /= 2
    assert vec == Matrix.vec([0.0, -0.375, 0.75])
    vec @= Matrix([[1, 2, 3]])
    assert vec == Matrix([[0.0, 0.0, 0.0], [-0.375, -0.375 * 2, -0.375 * 3], [0.75, 0.75 * 2, 0.75 * 3]])


def test_rowcolmatops():
    mat = Matrix.ones(3, 3)
    second_col = mat.col[1]
    second_row = mat.row[1]
    mat.col[1] *= 2
    assert mat == Matrix([[1, 2, 1], [1, 2, 1], [1, 2, 1]])
    assert list(second_col) == [2, 2, 2]
    assert list(second_row) == [1, 2, 1]
    mat.row[1] -= 1
    assert mat == Matrix([[1, 2, 1], [0, 1, 0], [1, 2, 1]])
    assert list(second_col) == [2, 1, 2]
    assert list(second_row) == [0, 1, 0]
    mat.row[0] *= -1
    assert mat == Matrix([[-1, -2, -1], [0, 1, 0], [1, 2, 1]])
    assert list(second_col) == [-2, 1, 2]
    assert list(second_row) == [0, 1, 0]
    mat.col[2] += 2
    assert mat == Matrix([[-1, -2, 1], [0, 1, 2], [1, 2, 3]])
    assert list(second_col) == [-2, 1, 2]
    assert list(second_row) == [0, 1, 2]
    mat.row[2] += 5
    assert mat == Matrix([[-1, -2, 1], [0, 1, 2], [6, 7, 8]])
    assert list(second_col) == [-2, 1, 7]
    assert list(second_row) == [0, 1, 2]
    mat.col[0] -= 1
    assert mat == Matrix([[-2, -2, 1], [-1, 1, 2], [5, 7, 8]])
    assert list(second_col) == [-2, 1, 7]
    assert list(second_row) == [-1, 1, 2]
    mat.col[1] //= 2
    assert mat == Matrix([[-2, -1, 1], [-1, 0, 2], [5, 3, 8]])
    assert list(second_col) == [-1, 0, 3]
    assert list(second_row) == [-1, 0, 2]
    mat.row[0] //= 2
    assert mat == Matrix([[-1, 0, 0], [-1, 0, 2], [5, 3, 8]])
    assert list(second_col) == [0, 0, 3]
    assert list(second_row) == [-1, 0, 2]
    mat.row[1] /= 2
    assert mat == Matrix([[-1, 0, 0], [-1, 0, 2], [5, 3, 8]])
    assert list(second_col) == [0, 0, 3]
    assert list(second_row) == [-0.5, 0, 1]
    mat.col[2] /= 2
    assert mat == Matrix([[-1, -1, 0], [-0.5, 0.0, 0.5], [5, 3, 4]])
    assert list(second_col) == [-1, 0.0, 3]
    assert list(second_row) == [-0.5, 0.0, 0.5]
