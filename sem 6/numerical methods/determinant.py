from matrix import Matrix
import random_gen

from typing import Iterable
import math
import itertools
import abc


class Determinant(abc.ABC):
    @abc.abstractmethod
    def calculate[T](cls, mat: Matrix[T]) -> T: ...


class DeterminantStupid(Determinant):
    def calculate[T](self, mat: Matrix[T]) -> T:
        return sum(
            self.signum(perm) * math.prod(mat[i, v] for i, v in enumerate(perm))
            for perm in itertools.permutations(range(mat.rows))
        )

    def signum(self, perm: tuple) -> int:
        return 1 if sum(perm[i] > perm[j] for i in range(len(perm)) for j in range(i + 1, len(perm))) % 2 == 0 else -1


def test_determinant_combinatorical():
    assert Matrix.eye(3).det(DeterminantStupid()) == 1
    assert Matrix.eye(2).det(DeterminantStupid()) == 1
    assert Matrix.eye(1).det(DeterminantStupid()) == 1

    for _ in range(10):
        diag = random_gen.generate_random_diagonal_matrix(size=5, low=1, value_type=int)
        upper = random_gen.generate_random_upper_triangular_matrix(size=5, low=1, value_type=int)
        lower = random_gen.generate_random_lower_triangular_matrix(size=5, low=1, value_type=int)
        assert diag.det(DeterminantStupid()) == math.prod(diag[i, i] for i in range(diag.cols))
        assert upper.det(DeterminantStupid()) == math.prod(upper[i, i] for i in range(diag.cols))
        assert lower.det(DeterminantStupid()) == math.prod(lower[i, i] for i in range(diag.cols))
        diag = random_gen.generate_random_diagonal_matrix(size=5, high=-1, value_type=int)
        upper = random_gen.generate_random_upper_triangular_matrix(size=5, high=-1, value_type=int)
        lower = random_gen.generate_random_lower_triangular_matrix(size=5, high=-1, value_type=int)
        assert diag.det(DeterminantStupid()) == math.prod(diag[i, i] for i in range(diag.cols))
        assert upper.det(DeterminantStupid()) == math.prod(upper[i, i] for i in range(diag.cols))
        assert lower.det(DeterminantStupid()) == math.prod(lower[i, i] for i in range(diag.cols))
    mat, det = random_gen.generate_mixed_matrix_with_known_determinant(size=9, span=range(1, 5))
    assert mat.det(DeterminantStupid()) == det
