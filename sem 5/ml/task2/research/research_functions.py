from collections import Counter
from typing import List


def are_multisets_equal(x: List[int], y: List[int]) -> bool:
    """
    Проверить, задают ли два вектора одно и то же мультимножество.
    """
    return Counter(x) == Counter(y)

def max_prod_mod_3(x: List[int]) -> int:
    """
    Вернуть максимальное прозведение соседних элементов в массиве x, 
    таких что хотя бы один множитель в произведении делится на 3.
    Если таких произведений нет, то вернуть -1.
    """
    return max((a * b for a, b in zip(x, x[1:]) if a % 3 == 0 or b % 3 == 0), default=-1)


def convert_image(image: List[List[List[float]]], weights: List[float]) -> List[List[float]]:
    """
    Сложить каналы изображения с указанными весами.
    """
    return [[sum(x * y for x, y in zip(px, weights)) for px in column] for column in image]


def rle_scalar(x: List[List[int]], y:  List[List[int]]) -> int:
    """
    Найти скалярное произведение между векторами x и y, заданными в формате RLE.
    В случае несовпадения длин векторов вернуть -1.
    """
    def take(rle_list: list[list[int]], idx: list[int]):
        while idx[1] < len(rle_list) and rle_list[idx[1]][1] <= 0:
            idx[1] += 1
        if idx[1] >= len(rle_list): return None
        val = rle_list[idx[1]][0]
        idx[0] += 1
        if idx[0] == rle_list[idx[1]][1]:
            idx[0] = 0
            idx[1] += 1
            if idx[1] >= len(rle_list):
                idx[0] = -1
        return val

    if not x or not y: return -1

    tmpx = [0, 0]
    tmpy = [0, 0]
    
    s = 0
    while tmpx[0] != -1 and tmpy[0] != -1:
        cx, cy = take(x, tmpx), take(y, tmpy)
        if cx is None or cy is None:
            return s if cx is cy else -1
        s += cx * cy
    return s if tmpx[0] == tmpy[0] == -1 else -1

def cosine_distance(X: List[List[float]], Y: List[List[float]]) -> List[List[float]]:
    """
    Вычислить матрицу косинусных расстояний между объектами X и Y. 
    В случае равенства хотя бы одно из двух векторов 0, косинусное расстояние считать равным 1.
    """
    def dot(x, y):
        return sum(a * b for a, b in zip(x, y))
    
    def zero(x):
        return all(c == 0 for c in x)
    
    return [[dot(x, y) / (dot(x, x) * dot(y, y)) ** 0.5
             if not zero(x) and not zero(y)
             else 1
             for y in Y] for x in X]
