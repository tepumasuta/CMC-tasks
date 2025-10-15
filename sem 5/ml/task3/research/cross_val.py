import numpy as np
import typing
from collections import defaultdict


def kfold_split(num_objects: int,
                num_folds: int) -> list[tuple[np.ndarray, np.ndarray]]:
    """Split [0, 1, ..., num_objects - 1] into equal num_folds folds
       (last fold can be longer) and returns num_folds train-val
       pairs of indexes.

    Parameters:
    num_objects: number of objects in train set
    num_folds: number of folds for cross-validation split

    Returns:
    list of length num_folds, where i-th element of list
    contains tuple of 2 numpy arrays, he 1st numpy array
    contains all indexes without i-th fold while the 2nd
    one contains i-th fold
    """
    indicies = np.arange(num_objects, dtype=np.int)
    s = num_objects // num_folds
    return (
        [(np.hstack([indicies[:i * s], indicies[(i + 1) * s:]]), indicies[i * s:(i + 1) * s])
         for i in range(num_folds - 1)] + [(indicies[:(num_folds - 1) * s], indicies[(num_folds - 1) * s:])]
    )


def knn_cv_score(X: np.ndarray, y: np.ndarray, parameters: dict[str, list],
                 score_function: callable,
                 folds: list[tuple[np.ndarray, np.ndarray]],
                 knn_class: object) -> dict[str, float]:
    """Takes train data, counts cross-validation score over
    grid of parameters (all possible parameters combinations)

    Parameters:
    X: train set
    y: train labels
    parameters: dict with keys from
        {n_neighbors, metrics, weights, normalizers}, values of type list,
        parameters['normalizers'] contains tuples (normalizer, normalizer_name)
        see parameters example in your jupyter notebook

    score_function: function with input (y_true, y_predict)
        which outputs score metric
    folds: output of kfold_split
    knn_class: class of knn model to fit

    Returns:
    dict: key - tuple of (normalizer_name, n_neighbors, metric, weight),
    value - mean score over all folds
    """
    stat = defaultdict(list)
    for normalizer, normalizer_name in parameters['normalizers']:
        for n_neighbors in parameters['n_neighbors']:
            for metric in parameters['metrics']:
                for weight in parameters['weights']:
                    for train_idx, test_idx in folds:
                        X_train, y_train = X[train_idx], y[train_idx]
                        X_test, y_test = X[test_idx], y[test_idx]

                        if normalizer is not None:
                            normalizer.fit(X_train)
                            X_train = normalizer.transform(X_train)
                            X_test = normalizer.transform(X_test)

                        knn = knn_class(n_neighbors=n_neighbors, metric=metric, weights=weight)
                        knn.fit(X_train, y_train)
                        key = (normalizer_name, n_neighbors, metric, weight)
                        stat[key].append(score_function(y_test, knn.predict(X_test)))
    result = {key: np.mean(value) for key, value in stat.items()}
    return result
