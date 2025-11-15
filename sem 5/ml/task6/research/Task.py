import numpy as np


class Preprocessor:
    def __init__(self):
        pass

    def fit(self, X, Y=None):
        pass

    def transform(self, X):
        pass

    def fit_transform(self, X, Y=None):
        pass


class MyOneHotEncoder(Preprocessor):
    def __init__(self, dtype=np.float64):
        super(Preprocessor).__init__()
        self.dtype = dtype

    def fit(self, X, Y=None):
        """
        param X: training objects, pandas-dataframe, shape [n_objects, n_features]
        param Y: unused
        """
        self.__categories = (X.copy()
                             .apply(lambda col: (u := np.unique(col)).size
                                    and dict(zip(u, np.arange(u.size, dtype=int))),
                                    axis=0))

    def transform(self, X):
        """
        param X: objects to transform, pandas-dataframe, shape [n_objects, n_features]
        returns: transformed objects, numpy-array, shape [n_objects, |f1| + |f2| + ...]
        """
        total_size = sum(len(categories) for categories in self.__categories)
        encoded = np.zeros((X.shape[0], total_size), dtype=self.dtype)

        def process_column(col_data, mapping):
            map_func = np.vectorize(lambda x: mapping[x])
            return map_func(col_data)

        cur_pos = 0
        for i, col in enumerate(X.columns):
            mapping = self.__categories.iloc[i]
            col_size = len(mapping)
            indices = process_column(X[col].values, mapping)
            row_indices = np.arange(X.shape[0])
            col_indices = cur_pos + indices
            encoded[row_indices, col_indices] = 1

            cur_pos += col_size

        return encoded

    def fit_transform(self, X, Y=None):
        self.fit(X)
        return self.transform(X)

    def get_params(self, deep=True):
        return {"dtype": self.dtype}


class SimpleCounterEncoder:
    def __init__(self, dtype=np.float64):
        self.dtype = dtype

    def fit(self, X, Y):
        """
        param X: training objects, pandas-dataframe, shape [n_objects, n_features]
        param Y: target for training objects, pandas-series, shape [n_objects,]
        """
        self.__categories = (X.copy()
                             .apply(lambda col: (u := np.unique(col)).size
                                    and {cat: (np.sum(Y[X[col.name] == cat]) / np.sum(X[col.name] == cat),
                                               np.sum(X[col.name] == cat) / X.shape[0])
                                    for cat in u}, axis=0))

    def transform(self, X, a=1e-5, b=1e-5):
        """
        param X: objects to transform, pandas-dataframe, shape [n_objects, n_features]
        param a: constant for counters, float
        param b: constant for counters, float
        returns: transformed objects, numpy-array, shape [n_objects, 3 * n_features]
        """
        def process_column(col_data, mapping):
            categories = list(mapping.keys())
            succ_values = np.array([mapping[cat][0] for cat in categories])
            count_values = np.array([mapping[cat][1] for cat in categories])

            cat_to_idx = {cat: idx for idx, cat in enumerate(categories)}
            indices = np.vectorize(lambda x: cat_to_idx[x])(col_data)

            succ_arr = succ_values[indices]
            count_arr = count_values[indices]
            combined_arr = (succ_arr + a) / (count_arr + b)

            return np.column_stack([succ_arr, count_arr, combined_arr])

        column_arrays = [process_column(X[col].values, self.__categories.iloc[i])
                         for i, col in enumerate(X.columns)]

        return np.hstack(column_arrays)

    def fit_transform(self, X, Y, a=1e-5, b=1e-5):
        self.fit(X, Y)
        return self.transform(X, a, b)

    def get_params(self, deep=True):
        return {"dtype": self.dtype}


def group_k_fold(size, n_splits=3, seed=1):
    idx = np.arange(size)
    np.random.seed(seed)
    idx = np.random.permutation(idx)
    n_ = size // n_splits
    for i in range(n_splits - 1):
        yield idx[i * n_: (i + 1) * n_], np.hstack((idx[:i * n_], idx[(i + 1) * n_:]))
    yield idx[(n_splits - 1) * n_:], idx[:(n_splits - 1) * n_]


class FoldCounters:
    def __init__(self, n_folds=3, dtype=np.float64):
        self.dtype = dtype
        self.n_folds = n_folds

    def fit(self, X, y, seed=0):
        """
        param X: training objects, pandas-dataframe, shape [n_objects, n_features]
        param Y: target for training objects, pandas-series, shape [n_objects,]
        param seed: random seed, int
        """
        n_samples = X.shape[0]
        self.__folds_assignments = np.zeros(n_samples, dtype=int)
        self.__encoders = [SimpleCounterEncoder() for _ in range(self.n_folds)]

        for fold_num, (test_idx, train_idx) in enumerate(group_k_fold(n_samples, self.n_folds, seed)):
            self.__folds_assignments[test_idx] = fold_num
            self.__encoders[fold_num].fit(X.iloc[train_idx], y.iloc[train_idx])

        return self

    def transform(self, X, a=1e-5, b=1e-5):
        """
        param X: objects to transform, pandas-dataframe, shape [n_objects, n_features]
        param a: constant for counters, float
        param b: constant for counters, float
        returns: transformed objects, numpy-array, shape [n_objects, 3 * n_features]
        """
        if self.__encoders is None:
            raise ValueError("Encoder not fitted")

        res = np.zeros((X.shape[0], 3 * len(X.columns)))

        for fold_num in range(self.n_folds):
            mask = self.__folds_assignments == fold_num
            X_fold = X.iloc[mask]
            res[mask] = self.__encoders[fold_num].transform(X_fold, a, b)
        return res

    def fit_transform(self, X, Y, a=1e-5, b=1e-5):
        self.fit(X, Y)
        return self.transform(X, a, b)


def weights(x, y):
    """
    param x: training set of one feature, numpy-array, shape [n_objects,]
    param y: target for training objects, numpy-array, shape [n_objects,]
    returns: optimal weights, numpy-array, shape [|x unique values|,]
    """
    _, inverse, counts = np.unique(x, return_inverse=True, return_counts=True)
    return np.bincount(inverse, weights=y) / counts
