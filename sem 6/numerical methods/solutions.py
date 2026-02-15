import matrix
import random_gen
import determinant

import abc
import dataclasses


class LinearSystemSolver(abc.ABC):
    @dataclasses.dataclass
    class Solution:
        solvable: bool
        solution: "matrix.Matrix | None" = None

    @abc.abstractmethod
    def solve(self, system: "matrix.Matrix", rhs: "matrix.Matrix") -> Solution: ...


class LinearSystemCramer(LinearSystemSolver):
    def __init__(self, det: "determinant.Determinant | None" = None):
        self.det = det

    def solve(self, system: "matrix.Matrix", rhs: "matrix.Matrix"):
        if system.cols != system.rows or system.rows != rhs.rows:
            assert False, "TODO: raise ValueError"
        det = system.det(self.det)
        if det == 0:
            return LinearSystemSolver.Solution(False)
        res = []
        for j in range(system.cols):
            mat = system.copy()
            mat.col[j] = rhs.col[0]
            res.append(mat.det(self.det) / det)
        return LinearSystemSolver.Solution(True, matrix.Matrix.vec(res))


def basic_test(solver: LinearSystemSolver):
    for _ in range(100):
        system = random_gen.generate_random_linear_system(size=4, low=-100, high=100)
        solution = solver.solve(matrix.Matrix.from_mat(system.system), matrix.Matrix.from_mat(system.rhs))
        assert solution.solution == system.solution
        assert solution.solvable
    for _ in range(100):
        system = random_gen.generate_random_linear_system(size=4, low=-100, high=100)
        solution = solver.solve(system.system, system.rhs)
        assert solution.solution == system.solution
        assert solution.solvable
    for _ in range(100):
        system = random_gen.generate_random_unsolvable_linear_system(size=4, low=-100, high=100)
        solution = solver.solve(system.system, system.rhs)
        assert not solution.solvable


def test_cramer():
    basic_test(LinearSystemCramer())
