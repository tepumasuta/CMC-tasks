import numpy as np


def are_multisets_equal(x: np.ndarray, y: np.ndarray) -> bool:
    """
    Проверить, задают ли два вектора одно и то же мультимножество.
    """
    return np.array_equiv(np.unique(x, return_counts=True), np.unique(y, return_counts=True))


def max_prod_mod_3(x: np.ndarray) -> int:
    """
    Вернуть максимальное прозведение соседних элементов в массиве x, 
    таких что хотя бы один множитель в произведении делится на 3.
    Если таких произведений нет, то вернуть -1.
    """
    divisible = x % 3 == 0
    ok_idxs = divisible[1:] | divisible[:-1]
    return (x[:-1] * x[1:])[ok_idxs].max(initial=-1)


def convert_image(image: np.ndarray, weights: np.ndarray) -> np.ndarray:
    """
    Сложить каналы изображения с указанными весами.
    """
    return np.tensordot(image, weights, axes=([2],[0]))


def rle_scalar(x: np.ndarray, y: np.ndarray) -> int:
    """
    Найти скалярное произведение между векторами x и y, заданными в формате RLE.
    В случае несовпадения длин векторов вернуть -1.
    """
    length_x = np.sum(x[:, 1])
    length_y = np.sum(y[:, 1])
    
    if length_x != length_y:
        return -1
    
    full_x = np.repeat(x[:, 0], x[:, 1])
    full_y = np.repeat(y[:, 0], y[:, 1])
    
    return np.dot(full_x, full_y)


def cosine_distance(X: np.ndarray, Y: np.ndarray) -> np.ndarray:
    """
    Вычислить матрицу косинусных расстояний между объектами X и Y.
    В случае равенства хотя бы одно из двух векторов 0, косинусное расстояние считать равным 1.
    """
    dot_prods = np.dot(X, Y.T)
    
    norm_X = np.linalg.norm(X, axis=1)
    norm_Y = np.linalg.norm(Y, axis=1)
    
    outer_norms = np.outer(norm_X, norm_Y)
    cos_sim = np.divide(dot_prods, outer_norms,
                        out=np.zeros_like(outer_norms),
                        where=outer_norms != 0)

    cos_sim = np.clip(cos_sim, -1, 1)

    norm_X_mask = (norm_X == 0)[..., np.newaxis]
    norm_Y_mask = (norm_Y == 0)[np.newaxis, ...]
    zero_vector_mask = norm_X_mask | norm_Y_mask
    cos_sim[zero_vector_mask] = 1.0
    
    return cos_sim
