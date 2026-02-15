import matrix
import random_gen

from typing import Iterable
import math
import itertools
import abc


class Determinant(abc.ABC):
    @abc.abstractmethod
    def calculate[T](self, mat: "matrix.Matrix[T]") -> T: ...


class DeterminantStupid(Determinant):
    def calculate[T](self, mat: "matrix.Matrix[T]") -> T:
        return sum(
            self.signum(perm) * math.prod(mat[i, v] for i, v in enumerate(perm))
            for perm in itertools.permutations(range(mat.rows))
        )

    def signum(self, perm: tuple) -> int:
        return 1 if sum(perm[i] > perm[j] for i in range(len(perm)) for j in range(i + 1, len(perm))) % 2 == 0 else -1


class DeterminantRecursive(Determinant):
    def calculate[T](self, mat: "matrix.Matrix[T]") -> T:
        if mat.cols == 1:
            return mat[0, 0]
        return sum((-1) ** i * self.calculate(mat.cross_out(row=0, col=i)) * mat[0, i] for i in range(mat.cols))


def basic_test(determinant: Determinant):
    assert matrix.Matrix.eye(3).det(determinant) == 1
    assert matrix.Matrix.eye(2).det(determinant) == 1
    assert matrix.Matrix.eye(1).det(determinant) == 1

    for _ in range(10):
        diag = random_gen.generate_random_diagonal_matrix(size=5, low=1, value_type=int)
        upper = random_gen.generate_random_upper_triangular_matrix(size=5, low=1, value_type=int)
        lower = random_gen.generate_random_lower_triangular_matrix(size=5, low=1, value_type=int)
        assert diag.det(determinant) == math.prod(diag[i, i] for i in range(diag.cols))
        assert upper.det(determinant) == math.prod(upper[i, i] for i in range(diag.cols))
        assert lower.det(determinant) == math.prod(lower[i, i] for i in range(diag.cols))
        diag = random_gen.generate_random_diagonal_matrix(size=5, high=-1, value_type=int)
        upper = random_gen.generate_random_upper_triangular_matrix(size=5, high=-1, value_type=int)
        lower = random_gen.generate_random_lower_triangular_matrix(size=5, high=-1, value_type=int)
        assert diag.det(determinant) == math.prod(diag[i, i] for i in range(diag.cols))
        assert upper.det(determinant) == math.prod(upper[i, i] for i in range(diag.cols))
        assert lower.det(determinant) == math.prod(lower[i, i] for i in range(diag.cols))
    mat, det = random_gen.generate_mixed_matrix_with_known_determinant(size=9, span=range(1, 5))
    assert mat.det(determinant) == det


def test_determinant_combinatorical():
    basic_test(DeterminantStupid())


def test_determinant_recursive():
    basic_test(DeterminantRecursive())


def test_coordination():
    for _ in range(10):
        mat, _ = random_gen.generate_mixed_matrix_with_known_determinant(size=7, span=range(1, 5))
        assert mat.det(DeterminantStupid()) == mat.det(DeterminantRecursive())
