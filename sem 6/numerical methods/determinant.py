from matrix import Matrix
import random_gen

from typing import Iterable
import math
import itertools
import abc


class Determinant(abc.ABC):
    @abc.abstractmethod
    def calculate[T](self, mat: Matrix[T]) -> T: ...


class DeterminantStupid(Determinant):
    def calculate[T](self, mat: Matrix[T]) -> T:
        return sum(
            self.signum(perm) * math.prod(mat[i, v] for i, v in enumerate(perm))
            for perm in itertools.permutations(range(mat.rows))
        )

    def signum(self, perm: tuple) -> int:
        return 1 if sum(perm[i] > perm[j] for i in range(len(perm)) for j in range(i + 1, len(perm))) % 2 == 0 else -1


class DeterminantRecursive(Determinant):
    def calculate[T](self, mat: Matrix[T]) -> T:
        return self.rec(mat, size=mat.cols)

    def rec[T](self, mat: Matrix[T], size: int) -> T: ...


def basic_test(determinant: Determinant):
    assert Matrix.eye(3).det(determinant) == 1
    assert Matrix.eye(2).det(determinant) == 1
    assert Matrix.eye(1).det(determinant) == 1

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
