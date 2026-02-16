import matrix
import random_gen

from typing import Iterable
import abc
import math
import functools


class SquareRootDecompostion[T](abc.ABC):
    def __init__(self, mat: "matrix.Matrix[T]"):
        self.mat = mat

    @abc.abstractmethod
    def is_applicable(self) -> bool: ...

    @abc.abstractmethod
    def decompose(self: "matrix.Matrix[T]") -> "Iterable[matrix.Matrix]": ...


class BasicSquareRootDecompostion[T](SquareRootDecompostion):
    def __init__(self, mat):
        super().__init__(mat)

    def is_applicable(self) -> bool:
        return self.mat.is_symmetric() and all(self.mat[i, i] != 0 for i in range(self.mat.cols))

    def decompose(self) -> "tuple[matrix.Matrix[T], matrix.Matrix[T], matrix.Matrix[T]]":
        s = matrix.Matrix(
            ((self.s(i, j) if j >= i else 0 for j in range(self.mat.cols)) for i in range(self.mat.rows)),
        )
        d = matrix.Matrix.diag([self.d(i) for i in range(self.mat.rows)])
        return s.T, d, s

    def signum(self, value) -> int:
        if value < 0:
            return -1
        if value > 0:
            return 1
        return 0

    @functools.cache
    def subexpr(self, i):
        return self.mat[i, i] - sum(abs(self.s(l, i)) ** 2 * self.d(l) for l in range(i))

    @functools.cache
    def d(self, i):
        return self.signum(self.subexpr(i))

    @functools.cache
    def s(self, i, j):
        if i == j:
            return math.sqrt(abs(self.subexpr(i)))
        return (self.mat[i, j] - sum(self.s(l, i) * self.s(l, j) * self.d(l) for l in range(i))) / (
            self.s(i, i) * self.d(i)
        )


def test_basicsqrtdec():
    for s in range(1, 10):
        m = random_gen.generate_random_matrix(size=s, low=-100, high=100)
        assert BasicSquareRootDecompostion(m).is_applicable() == m.is_symmetric()
    for _ in range(6):
        for size in range(1, 7):
            m = random_gen.generate_random_symmetric_matrix(size=size, low=-100, high=100)
            decomposable = all(m[i, i] != 0 for i in range(size))
            dec = BasicSquareRootDecompostion(m)
            assert dec.is_applicable() == decomposable
            if not decomposable:
                continue
            left, diag, right = dec.decompose()
            assert left.is_lower_triangular()
            assert right.is_upper_triangular()
            assert diag.is_diag()
            assert left.T == right
            assert left @ diag @ right == m
    for _ in range(6):
        for size in range(1, 7):
            m = random_gen.generate_random_positively_definite_matrix(size=size, low=-100, high=100)
            decomposable = all(m[i, i] != 0 for i in range(size))
            dec = BasicSquareRootDecompostion(m)
            assert dec.is_applicable() == decomposable
            if not decomposable:
                continue
            left, diag, right = dec.decompose()
            assert left.is_lower_triangular()
            assert right.is_upper_triangular()
            assert diag.is_diag()
            assert left.T == right
            assert left @ diag @ right == m
