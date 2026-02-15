import determinant

from collections.abc import Sized
from typing import Iterable, Callable
import copy
import itertools
import functools
import math
import weakref
from fractions import Fraction


class Matrix[U]:
    class Row:
        def __init__(self, mat: "weakref.ProxyType[Matrix[U]]", index: int):
            self.mat: Matrix[U] = mat
            self.index = index

        def __len__(self) -> int:
            return self.mat.cols

        def __iter__(self) -> Iterable[U]:
            yield from (self.mat[self.index, j] for j in range(self.mat.cols))

        def __iadd__(self, other):
            if isinstance(other, (int, float, Fraction)):
                for j in range(self.mat.cols):
                    self.mat[self.index, j] += other
                return self
            return NotImplemented

        def __isub__(self, other):
            if isinstance(other, (int, float, Fraction)):
                for j in range(self.mat.cols):
                    self.mat[self.index, j] -= other
                return self
            return NotImplemented

        def __imul__(self, other):
            if isinstance(other, (int, float, Fraction)):
                for j in range(self.mat.cols):
                    self.mat[self.index, j] *= other
                return self
            return NotImplemented

        def __itruediv__(self, other):
            if isinstance(other, (int, float, Fraction)):
                for j in range(self.mat.cols):
                    self.mat[self.index, j] /= other
                return self
            return NotImplemented

        def __ifloordiv__(self, other):
            if isinstance(other, int):
                for j in range(self.mat.cols):
                    self.mat[self.index, j] //= other
                return self
            return NotImplemented

    class Col:
        def __init__(self, mat: "weakref.ProxyType[Matrix[U]]", index: int):
            self.mat: Matrix[U] = mat
            self.index = index

        def __len__(self) -> int:
            return self.mat.rows

        def __iter__(self) -> Iterable[U]:
            yield from (self.mat[i, self.index] for i in range(self.mat.rows))

        def __iadd__(self, other):
            if isinstance(other, (int, float, Fraction)):
                for i in range(self.mat.rows):
                    self.mat[i, self.index] += other
                return self
            return NotImplemented

        def __isub__(self, other):
            if isinstance(other, (int, float, Fraction)):
                for i in range(self.mat.rows):
                    self.mat[i, self.index] -= other
                return self
            return NotImplemented

        def __imul__(self, other):
            if isinstance(other, (int, float, Fraction)):
                for i in range(self.mat.rows):
                    self.mat[i, self.index] *= other
                return self
            return NotImplemented

        def __itruediv__(self, other):
            if isinstance(other, (int, float, Fraction)):
                for i in range(self.mat.rows):
                    self.mat[i, self.index] /= other
                return self
            return NotImplemented

        def __ifloordiv__(self, other):
            if isinstance(other, int):
                for i in range(self.mat.rows):
                    self.mat[i, self.index] //= other
                return self
            return NotImplemented

    class RowsView:
        def __init__(self, mat: "Matrix"):
            self.base = weakref.proxy(mat)

        def __getitem__(self, index) -> "Matrix.Row":
            if isinstance(index, int):
                return Matrix.Row(self.base, index)
            assert False, "TODO: raise KeyError"

        def __setitem__(self, index, value: Iterable[U]):
            if isinstance(value, Matrix.Row) and value.index == index and self.base == value.mat:
                return
            if not isinstance(value, Sized) or isinstance(value, Matrix.Col) and value.mat is self.base:
                value = tuple(value)
            if self.base.rows != len(value):
                assert False, "TODO: raise ValueError"
            for j, v in enumerate(value):
                self.base[index, j] = v

    class ColsView:
        def __init__(self, mat: "Matrix"):
            self.base: Matrix = weakref.proxy(mat)

        def __getitem__(self, index) -> "Matrix.Col":
            if isinstance(index, int):
                return Matrix.Col(self.base, index)
            assert False, "TODO: raise KeyError"

        def __setitem__(self, index, value: Iterable[U]):
            if isinstance(value, Matrix.Col) and value.index == index and self.base == value.mat:
                return
            if not isinstance(value, Sized) or isinstance(value, Matrix.Row) and value.mat is self.base:
                value = tuple(value)
            if self.base.rows != len(value):
                assert False, "TODO: raise ValueError"
            for i, v in enumerate(value):
                self.base[i, index] = v

    def __init__(self, raw_matrix: Iterable[Iterable[U]], *, convert_to_fractions=True):
        self.__raw: list[list[U]] = list(map(list, raw_matrix))
        if not self.__raw or not self.__raw[0]:
            assert False, "TODO: raise ValueError"
        self.__rows = len(self.__raw)
        self.__cols = len(self.__raw[0])
        if not all(len(row) == self.__cols for row in self.__raw):
            assert False, "TODO: raise ValueError"
        self.__type = functools.reduce(Matrix.__merge_types, map(type, itertools.chain.from_iterable(self.__raw)))
        if convert_to_fractions and self.generic_type is not float:
            self.__raw = list(list(map(Fraction, row)) for row in self.__raw)
            self.__type = Fraction

    def copy(self) -> "Matrix[U]":
        return Matrix(copy.deepcopy(self.__raw), convert_to_fractions=False)

    @property
    def rows(self) -> int:
        return self.__rows

    @property
    def cols(self) -> int:
        return self.__cols

    @property
    def generic_type(self) -> U:
        return self.__type

    @property
    def T(self) -> "Matrix[T]":
        return Matrix(zip(*self.__raw), convert_to_fractions=False)

    @property
    def row(self) -> RowsView:
        return Matrix.RowsView(self)

    @property
    def col(self) -> ColsView:
        return Matrix.ColsView(self)

    def __add__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(
                ((self[i, j] + other[i, j] for j in range(self.cols)) for i in range(self.rows)),
                convert_to_fractions=False,
            )
        if isinstance(other, (int, float, Fraction)):
            return Matrix(
                ((self[i, j] + other for j in range(self.cols)) for i in range(self.rows)), convert_to_fractions=False
            )
        return NotImplemented

    def __radd__(self, other) -> "Matrix":
        if isinstance(other, (int, float, Fraction)):
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
            self.__type = self.__merge_types(self.generic_type, other.generic_type)
            return self
        if isinstance(other, (int, float, Fraction)):
            self.__raw = [[self[i, j] + other for j in range(self.cols)] for i in range(self.rows)]
            self.__type = self.__merge_types(self.generic_type, type(other))
            return self
        return NotImplemented

    def __sub__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(
                ((self[i, j] - other[i, j] for j in range(self.cols)) for i in range(self.rows)),
                convert_to_fractions=False,
            )
        if isinstance(other, (int, float, Fraction)):
            return Matrix(
                ((self[i, j] - other for j in range(self.cols)) for i in range(self.rows)), convert_to_fractions=False
            )
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
            self.__type = self.__merge_types(self.generic_type, other.generic_type)
            return self
        if isinstance(other, (int, float, Fraction)):
            self.__raw = [[self[i, j] - other for j in range(self.cols)] for i in range(self.rows)]
            self.__type = self.__merge_types(self.generic_type, type(other))
            return self
        return NotImplemented

    def __mul__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(
                ((self[i, j] * other[i, j] for j in range(self.cols)) for i in range(self.rows)),
                convert_to_fractions=False,
            )
        if isinstance(other, (int, float, Fraction)):
            return Matrix(((v * other for v in row) for row in self.__raw), convert_to_fractions=False)
        return NotImplemented

    def __rmul__(self, other) -> "Matrix":
        if isinstance(other, (int, float, Fraction)):
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
            self.__type = self.__merge_types(self.generic_type, other.generic_type)
            return self
        if isinstance(other, (int, float, Fraction)):
            self.__raw = [[v * other for v in row] for row in self.__raw]
            self.__type = self.__merge_types(self.generic_type, type(other))
            return self
        return NotImplemented

    def __truediv__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(
                ((self[i, j] / other[i, j] for j in range(self.cols)) for i in range(self.rows)),
                convert_to_fractions=False,
            )
        if isinstance(other, (int, float, Fraction)):
            return Matrix(((v / other for v in row) for row in self.__raw), convert_to_fractions=False)
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
            self.__type = self.__merge_types(self.generic_type, other.generic_type)
            return self
        if isinstance(other, (int, float, Fraction)):
            self.__raw = [[v / other for v in row] for row in self.__raw]
            self.__type = self.__merge_types(self.generic_type, type(other))
            return self
        return NotImplemented

    def __floordiv__(self, other) -> "Matrix":
        if isinstance(other, Matrix):
            if self.cols != other.cols or self.rows != other.rows:
                assert False, "TODO: raise ValueError"
            return Matrix(
                ((self[i, j] // other[i, j] for j in range(self.cols)) for i in range(self.rows)),
                convert_to_fractions=False,
            )
        if isinstance(other, int):
            return Matrix(((v // other for v in row) for row in self.__raw), convert_to_fractions=False)
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
                (
                    (sum(self[i, k] * other[k, j] for k in range(self.cols)) for j in range(other.cols))
                    for i in range(self.rows)
                ),
                convert_to_fractions=False,
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
            self.__type = self.__merge_types(self.generic_type, other.generic_type)
            return self
        return NotImplemented

    def __getitem__(self, index: tuple[int, int]) -> U:
        i, j = index
        if i not in range(self.rows) or j not in range(self.cols):
            assert False, "TODO: raise KeyError"
        return self.__raw[i][j]

    def __setitem__(self, index: tuple[int, int], value) -> U:
        i, j = index
        if i not in range(self.rows) or j not in range(self.cols):
            assert False, "TODO: raise KeyError"
        self.__raw[i][j] = value
        self.__type = self.__merge_types(self.generic_type, type(value))

    def __eq__(self, other) -> bool:
        if not isinstance(other, Matrix):
            return False
        cmp = self.__get_cmp(self.__merge_types(self.generic_type, other.generic_type))
        return (
            self.cols == other.cols
            and self.rows == other.rows
            and all(cmp(self[i, j], other[i, j]) for i, j in itertools.product(range(self.rows), range(self.cols)))
        )

    def __repr__(self) -> str:
        max_spacing = max(len(str(v)) for row in self.__raw for v in row)
        return f'{'\n'.join(f'[{' '.join(str(v).rjust(max_spacing)for v in row)}]' for row in self.__raw)}'

    def swap_rows(self, i, j):
        if i == j:
            return
        self.__raw[i], self.__raw[j] = self.__raw[j], self.__raw[i]

    def swap_cols(self, i, j):
        if i == j:
            return
        for k in range(self.rows):
            self[k, i], self[k, j] = self[k, j], self[k, i]

    def cross_out(self, *, col=None, row=None) -> "Matrix[U]":
        if col is None and row is None:
            assert False, "TODO: raise ValueError"
        if col is not None and col not in range(self.cols):
            assert False, "TODO: raise ValueError"
        if row is not None and row not in range(self.rows):
            assert False, "TODO: raise ValueError"
        if col is None:
            return Matrix((self.row[i] for i in range(self.rows) if i != row), convert_to_fractions=False)
        if row is None:
            return Matrix((self.col[j] for j in range(self.cols) if j != col), convert_to_fractions=False).T
        return Matrix(
            ((self[i, j] for j in range(self.cols) if j != col) for i in range(self.rows) if i != row),
            convert_to_fractions=False,
        )

    def det(self, alg: "determinant.Determinant | None" = None) -> U:
        if self.cols != self.rows:
            assert False, "TODO: raise ValueError"
        if alg is None:
            alg = determinant.DeterminantStupid()
        return alg.calculate(self)

    def minor(
        self,
        *,
        rows: Iterable[int] | None = None,
        cols: Iterable[int] | None = None,
        alg: "determinant.Determinant | None" = None,
    ) -> U:
        rows = set(rows if rows is not None else range(self.rows))
        cols = set(cols if cols is not None else range(self.cols))
        if not rows or not cols:
            assert False, "TODO: raise ValueError"

        return Matrix([[self[i, j] for j in range(self.cols) if j in cols] for i in range(self.rows) if i in rows]).det(
            alg
        )

    def is_unit(self) -> bool:
        cmp = self.__get_cmp(self.generic_type)
        return self.is_diag() and all(cmp(self[i, i], 1) for i in range(self.rows))

    def is_diag(self) -> bool:
        cmp = self.__get_cmp(self.generic_type)
        return self.cols == self.rows and all(
            cmp(self[i, j], 0) for i, j in itertools.product(range(self.cols), repeat=2) if i != j
        )

    def is_symmetric(self) -> bool:
        cmp = self.__get_cmp(self.generic_type)
        return all(cmp(self[i, j], self[j, i]) for i in range(self.rows) for j in range(i + 1, self.cols))

    def is_upper_triangular(self) -> bool:
        cmp = self.__get_cmp(self.generic_type)
        return self.cols == self.rows and all(
            cmp(self[i, j], 0) for j in range(self.cols) for i in range(j + 1, self.rows)
        )

    def is_lower_triangular(self) -> bool:
        cmp = self.__get_cmp(self.generic_type)
        return self.cols == self.rows and all(
            cmp(self[i, j], 0) for i in range(self.rows) for j in range(i + 1, self.cols)
        )

    def is_positively_definite(self, det_alg: "determinant.Determinant | None" = None) -> bool | None:
        if not self.is_symmetric():
            return None
        return all(self.minor(cols=range(size), rows=range(size), alg=det_alg) > 0 for size in range(1, self.cols + 1))

    @staticmethod
    def __merge_types(t1: type, t2: type) -> type:
        if t1 is float and t2 is float:
            return float
        if t1 is float and t2 is int:
            return float
        if t1 is int and t2 is float:
            return float
        if t1 is float and t2 is Fraction:
            return float
        if t1 is Fraction and t2 is float:
            return float
        if t1 is Fraction and t2 is Fraction:
            return Fraction
        if t1 is int and t2 is Fraction:
            return Fraction
        if t1 is Fraction and t2 is int:
            return Fraction
        if t1 is int and t2 is int:
            return int
        assert False, "TODO: raise ValueError"

    @staticmethod
    def __get_cmp(t) -> Callable[[U, U], bool]:
        if t is float:
            return lambda x, y: math.isclose(x, y)
        if issubclass(t, int) or t is Fraction:
            return lambda x, y: x == y
        assert False, "TODO: raise ValueError"

    @staticmethod
    def vec(vec: Iterable[U], *, convert_to_fractions=True) -> "Matrix[U]":
        return Matrix([[v] for v in vec], convert_to_fractions=convert_to_fractions)

    @staticmethod
    def zero(rows: int, cols: int, *, convert_to_fractions=True) -> "Matrix[U]":
        return Matrix([[0] * cols for _ in range(rows)], convert_to_fractions=convert_to_fractions)

    @staticmethod
    def ones(rows: int, cols: int, *, convert_to_fractions=True) -> "Matrix[U]":
        return Matrix([[1] * cols for _ in range(rows)], convert_to_fractions=convert_to_fractions)

    @staticmethod
    def diag(vec: Iterable[U], *, convert_to_fractions=True) -> "Matrix[U]":
        vec = tuple(vec)
        return Matrix(
            ((0,) * i + (v,) + (0,) * (len(vec) - i - 1) for i, v in enumerate(vec)),
            convert_to_fractions=convert_to_fractions,
        )

    @staticmethod
    def eye(size: int, *, convert_to_fractions=True) -> "Matrix[U]":
        return Matrix.diag((1,) * size, convert_to_fractions=convert_to_fractions)

    @staticmethod
    def from_mat(other: "Matrix[U]", *, convert_to_fractions=True) -> "Matrix[U]":
        return Matrix(other.__raw, convert_to_fractions=convert_to_fractions)


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
    assert Matrix.from_mat(Matrix([[1, 2], [3, 4]])) == Matrix([[1, 2], [3, 4]])
    mat = Matrix([[1, 2], [3, 4]])
    cp = mat.from_mat(mat)
    mat.row[0] *= 666
    assert cp == Matrix([[1, 2], [3, 4]]) != mat


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

    assert Matrix([[1, 2], [3, 4]]) * Fraction(2) == Matrix([[2, 4], [6, 8]])
    assert Fraction(2) * Matrix([[1, 2], [3, 4]]) == Matrix([[2, 4], [6, 8]])
    assert Matrix([[1, 2], [3, 4]]) * Fraction(1, 2) == Matrix([[Fraction(1, 2), 1], [Fraction(3, 2), 2]])
    assert Fraction(1, 2) * Matrix([[1, 2], [3, 4]]) == Matrix([[Fraction(1, 2), 1], [Fraction(3, 2), 2]])
    assert Matrix([[1, 2], [3, 4]]) / Fraction(2) == Matrix([[Fraction(1, 2), 1], [Fraction(3, 2), 2]])
    assert Matrix([[1, 2], [3, 4]]) / Fraction(1, 2) == Matrix([[2, 4], [6, 8]])

    assert Matrix([[1, 2], [3, 4]]) + 0 == Matrix([[1, 2], [3, 4]])
    assert 0 + Matrix([[1, 2], [3, 4]]) == Matrix([[1, 2], [3, 4]])
    assert Matrix([[1, 2], [3, 4]]) + 1 == Matrix([[2, 3], [4, 5]])
    assert Matrix([[1, 2], [3, 4]]) - 1 == Matrix([[0, 1], [2, 3]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) + 0.3 == Matrix([[1.3, 2.3, 3.3], [4.3, 5.3, 6.3]])
    assert 0.3 + Matrix([[1, 2, 3], [4, 5, 6]]) == Matrix([[1.3, 2.3, 3.3], [4.3, 5.3, 6.3]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) - 0.3 == Matrix([[0.7, 1.7, 2.7], [3.7, 4.7, 5.7]])

    assert Matrix([[1, 2], [3, 4]]) + Fraction() == Matrix([[1, 2], [3, 4]])
    assert Fraction() + Matrix([[1, 2], [3, 4]]) == Matrix([[1, 2], [3, 4]])
    assert Matrix([[1, 2], [3, 4]]) + Fraction(1) == Matrix([[2, 3], [4, 5]])
    assert Matrix([[1, 2], [3, 4]]) - Fraction(1) == Matrix([[0, 1], [2, 3]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) + Fraction(3, 10) == Matrix(
        [[Fraction(13, 10), Fraction(23, 10), Fraction(33, 10)], [Fraction(43, 10), Fraction(53, 10), Fraction(63, 10)]]
    )
    assert Fraction(3, 10) + Matrix([[1, 2, 3], [4, 5, 6]]) == Matrix(
        [[Fraction(13, 10), Fraction(23, 10), Fraction(33, 10)], [Fraction(43, 10), Fraction(53, 10), Fraction(63, 10)]]
    )
    assert Matrix([[1, 2, 3], [4, 5, 6]]) - Fraction(3, 10) == Matrix(
        [[Fraction(7, 10), Fraction(17, 10), Fraction(27, 10)], [Fraction(37, 10), Fraction(47, 10), Fraction(57, 10)]]
    )


def test_matops():
    assert Matrix([[1, 2], [3, 4]]) + Matrix([[5, 6], [7, 8]]) == Matrix([[6, 8], [10, 12]])
    assert Matrix([[1, 2], [3, 4]]) - Matrix.eye(2) == Matrix([[0, 2], [3, 3]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) + Matrix.ones(2, 3) == Matrix([[2, 3, 4], [5, 6, 7]])
    assert Matrix([[1, 2, 3], [4, 5, 6]]) - Matrix.ones(2, 3) == Matrix([[0, 1, 2], [3, 4, 5]])
    assert Matrix([[1, 2], [3, 4]]) * Matrix([[5, 6], [7, 8]]) == Matrix([[5, 12], [21, 32]])
    assert Matrix([[1, 2], [3, 4]]) / Matrix([[2, 2], [2, 2]]) == Matrix([[0.5, 1.0], [1.5, 2.0]])
    assert Matrix([[1, 2], [3, 4]]) // Matrix([[2, 2], [2, 2]]) == Matrix([[0, 1], [1, 2]])


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

    vec = Matrix.vec([1, 2, 3])
    vec += Fraction(1)
    assert vec == Matrix.vec([2, 3, 4])
    vec -= Fraction(1)
    assert vec == Matrix.vec([1, 2, 3])
    vec *= Fraction(2)
    assert vec == Matrix.vec([2, 4, 6])
    vec /= Fraction(2)
    assert vec == Matrix.vec([1, 2, 3])


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
    assert mat == Matrix([[-1, -1, 0], [-1, 0, 2], [5, 3, 8]])
    assert list(second_col) == [-1, 0, 3]
    assert list(second_row) == [-1, 0, 2]
    mat.row[1] /= 2
    assert mat == Matrix([[-1, -1, 0], [-0.5, 0, 1], [5, 3, 8]])
    assert list(second_col) == [-1, 0, 3]
    assert list(second_row) == [-0.5, 0, 1]
    mat.col[2] /= 2
    assert mat == Matrix([[-1, -1, 0], [-0.5, 0.0, 0.5], [5, 3, 4]])
    assert list(second_col) == [-1, 0.0, 3]
    assert list(second_row) == [-0.5, 0.0, 0.5]


def test_rowcolmatinsert():
    mat = Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
    mat.row[2] = mat.col[0]
    assert mat == Matrix([[1, 2, 3], [4, 5, 6], [1, 4, 7]])
    mat.col[1] = mat.row[0]
    assert mat == Matrix([[1, 1, 3], [4, 2, 6], [1, 3, 7]])
    mat.row[0] = mat.col[2]
    assert mat == Matrix([[3, 6, 7], [4, 2, 6], [1, 3, 7]])
    mat.col[2] = mat.col[1]
    assert mat == Matrix([[3, 6, 6], [4, 2, 2], [1, 3, 3]])
    mat.row[1] = [10, 20, 30]
    assert mat == Matrix([[3, 6, 6], [10, 20, 30], [1, 3, 3]])
    mat.col[0] = (40, 50, 60)
    assert mat == Matrix([[40, 6, 6], [50, 20, 30], [60, 3, 3]])
    mat.row[2] = iter([70, 80, 90])
    assert mat == Matrix([[40, 6, 6], [50, 20, 30], [70, 80, 90]])
    mat.col[1] = range(100, 103)
    assert mat == Matrix([[40, 100, 6], [50, 101, 30], [70, 102, 90]])
    mat.row[0] = (x * 2 for x in range(3))
    assert mat == Matrix([[0, 2, 4], [50, 101, 30], [70, 102, 90]])
    row1_ref = mat.row[1]
    mat.row[1] = mat.row[1]
    assert mat == Matrix([[0, 2, 4], [50, 101, 30], [70, 102, 90]])
    assert list(row1_ref) == [50, 101, 30]
    col2_ref = mat.col[2]
    mat.col[2] = mat.col[2]
    assert mat == Matrix([[0, 2, 4], [50, 101, 30], [70, 102, 90]])
    assert list(col2_ref) == [4, 30, 90]
    mat.row[0] = mat.col[0]
    assert mat == Matrix([[0, 50, 70], [50, 101, 30], [70, 102, 90]])
    mat.col[1] = mat.row[2]
    assert mat == Matrix([[0, 70, 70], [50, 102, 30], [70, 90, 90]])

    mat = Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
    mat.row[0] += Fraction(1)
    assert mat == Matrix([[2, 3, 4], [4, 5, 6], [7, 8, 9]])
    mat.row[0] -= Fraction(1)
    assert mat == Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
    mat.row[0] *= Fraction(2)
    assert mat == Matrix([[2, 4, 6], [4, 5, 6], [7, 8, 9]])
    mat.row[0] /= Fraction(2)
    assert mat == Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
    mat.col[0] += Fraction(1)
    assert mat == Matrix([[2, 2, 3], [5, 5, 6], [8, 8, 9]])
    mat.col[0] -= Fraction(1)
    assert mat == Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
    mat.col[0] *= Fraction(2)
    assert mat == Matrix([[2, 2, 3], [8, 5, 6], [14, 8, 9]])
    mat.col[0] /= Fraction(2)
    assert mat == Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]])


def test_swap():
    mat = Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
    mat.swap_rows(0, 2)
    assert mat == Matrix([[7, 8, 9], [4, 5, 6], [1, 2, 3]])
    mat.swap_cols(2, 1)
    assert mat == Matrix([[7, 9, 8], [4, 6, 5], [1, 3, 2]])
    mat.swap_rows(1, 1)
    assert mat == Matrix([[7, 9, 8], [4, 6, 5], [1, 3, 2]])
    mat.swap_cols(0, 0)
    assert mat == Matrix([[7, 9, 8], [4, 6, 5], [1, 3, 2]])
    mat.swap_rows(0, 1)
    assert mat == Matrix([[4, 6, 5], [7, 9, 8], [1, 3, 2]])
    mat.swap_cols(0, 2)
    assert mat == Matrix([[5, 6, 4], [8, 9, 7], [2, 3, 1]])
    mat.swap_cols(0, 2)
    mat.swap_cols(0, 2)
    assert mat == Matrix([[5, 6, 4], [8, 9, 7], [2, 3, 1]])


def test_special_matricies_checks():
    assert Matrix.eye(3).is_unit()
    assert Matrix.eye(3).is_diag()
    assert Matrix.eye(3).is_symmetric()
    assert Matrix.eye(3).is_upper_triangular()
    assert Matrix.eye(3).is_lower_triangular()

    assert Matrix([[1.0, 0, 0], [0, 1.0, 0], [0, 0, 3 / 3]]).is_unit()
    assert Matrix([[1.0, 0, 0], [0, 1.0, 0], [0, 0, 3 / 3]]).is_diag()
    assert Matrix([[1.0, 0, 0], [0, 1.0, 0], [0, 0, 3 / 3]]).is_symmetric()
    assert Matrix([[1.0, 0, 0], [0, 1.0, 0], [0, 0, 3 / 3]]).is_upper_triangular()
    assert Matrix([[1.0, 0, 0], [0, 1.0, 0], [0, 0, 3 / 3]]).is_lower_triangular()

    assert not Matrix.diag([1, 2, 3]).is_unit()
    assert Matrix.diag([1, 2, 3]).is_diag()
    assert Matrix.diag([1, 2, 3]).is_symmetric()
    assert Matrix.diag([1, 2, 3]).is_upper_triangular()
    assert Matrix.diag([1, 2, 3]).is_lower_triangular()

    assert not Matrix.diag([-1, -2, -3]).is_unit()
    assert Matrix.diag([-1, -2, -3]).is_diag()
    assert Matrix.diag([-1, -2, -3]).is_symmetric()
    assert Matrix.diag([-1, -2, -3]).is_upper_triangular()
    assert Matrix.diag([-1, -2, -3]).is_lower_triangular()

    assert not Matrix([[1, 2], [0, 3]]).is_unit()
    assert not Matrix([[1, 2], [0, 3]]).is_diag()
    assert not Matrix([[1, 2], [0, 3]]).is_symmetric()
    assert Matrix([[1, 2], [0, 3]]).is_upper_triangular()
    assert not Matrix([[1, 2], [0, 3]]).is_lower_triangular()

    assert not Matrix([[1, 2], [0, 3]]).T.is_unit()
    assert not Matrix([[1, 2], [0, 3]]).T.is_symmetric()
    assert not Matrix([[1, 2], [0, 3]]).T.is_upper_triangular()
    assert Matrix([[1, 2], [0, 3]]).T.is_lower_triangular()

    assert not Matrix([[1, 2], [2, 3]]).is_unit()
    assert Matrix([[1, 2], [2, 3]]).is_symmetric()
    assert Matrix([[1, 2], [2, 3]]).T.is_symmetric()
    assert not Matrix([[1, 2], [2, 3]]).is_upper_triangular()
    assert not Matrix([[1, 2], [2, 3]]).is_lower_triangular()

    assert not Matrix([[1, 2], [2, 3]]).is_unit()
    assert Matrix([[1, 2], [2, 3]]).is_symmetric()
    assert Matrix([[1, 2], [2, 3]]).T.is_symmetric()
    assert not Matrix([[1, 2], [2, 3]]).is_upper_triangular()
    assert not Matrix([[1, 2], [2, 3]]).is_lower_triangular()

    assert Matrix([[1, 10**9 / 10**9], [1000 / 1000, 3]]).is_symmetric()
    assert not Matrix([[1, 2], [1 / 10**15, 3]]).is_upper_triangular()
    assert not Matrix([[1, 10**-15], [2, 3]]).is_lower_triangular()


def test_positive_definiteness():
    assert Matrix.eye(3).is_positively_definite()
    assert Matrix([[1.0, 0, 0], [0, 1.0, 0], [0, 0, 3 / 3]]).is_positively_definite()
    assert Matrix.diag([1, 2, 3]).is_positively_definite()
    assert not Matrix.diag([-1, -1]).is_positively_definite()
    assert Matrix([[1, 2], [0, 3]]).is_positively_definite() is None
    assert Matrix([[1, 2], [0, 3]]).T.is_positively_definite() is None
    assert not Matrix([[1, 2], [2, 3]]).is_positively_definite()
    assert Matrix([[2, -1], [-1, 2]]).is_positively_definite()
    assert not Matrix([[1, 2], [2, 1]]).is_positively_definite()
    assert not Matrix.diag([-1, -1]).is_positively_definite()
    assert not Matrix([[3, 2, 0], [2, 2, 2], [0, 2, 1]]).is_positively_definite()


def test_crossoutmat():
    assert Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]]).cross_out(row=0, col=2) == Matrix([[4, 5], [7, 8]])
    assert Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]]).cross_out(row=0, col=2) == Matrix([[4, 5], [7, 8]])
    assert Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]]).cross_out(col=0) == Matrix([[2, 3], [5, 6], [8, 9]])
