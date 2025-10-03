import numpy as np
import typing


class MinMaxScaler:
    def fit(self, data: np.ndarray) -> None:
        """Store calculated statistics

        Parameters:
        data: train set, size (num_obj, num_features)
        """
        self.__min = data.min(axis=0)
        self.__max = data.max(axis=0)

    def transform(self, data: np.ndarray) -> np.ndarray:
        """
        Parameters:
        data: train set, size (num_obj, num_features)

        Return:
        scaled data, size (num_obj, num_features)
        """
        scalars = (self.__max - self.__min)[np.newaxis, :]

        return np.divide(data - self.__min[np.newaxis, :], scalars,
                         out=np.zeros_like(data, dtype=np.float64),
                         where=scalars != 0)


class StandardScaler:
    def fit(self, data: np.ndarray) -> None:
        """Store calculated statistics

        Parameters:
        data: train set, size (num_obj, num_features)
        """
        self.__mean = data.mean(axis=0)
        self.__std = data.std(axis=0)

    def transform(self, data: np.ndarray) -> np.ndarray:
        """
        Parameters:
        data: train set, size (num_obj, num_features)

        Return:
        scaled data, size (num_obj, num_features)
        """
        return np.divide(data - self.__mean, self.__std,
                         out=np.zeros_like(data, dtype=np.float64),
                         where=self.__std != 0)
