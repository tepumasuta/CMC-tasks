import numpy as np
from sklearn.svm import SVC
from sklearn.preprocessing import RobustScaler
from sklearn.feature_selection import SelectKBest
from sklearn.pipeline import Pipeline


def train_svm_and_predict(train_features, train_target, test_features):
    """
    train_features: np.array, (num_elements_train x num_features) - train data description, the same features and the same order as in train data
    train_target: np.array, (num_elements_train) - train data target
    test_features: np.array, (num_elements_test x num_features) -- some test data, features are in the same order as train features

    return: np.array, (num_elements_test) - test data predicted target, 1d array
    """
    pipeline_params = {
        'scaler': RobustScaler(),
        'feature_selection': SelectKBest(k=4),
        'svm__C': 1000,
        'svm__gamma': 0.01,
        'svm__kernel': 'rbf'
    }

    steps = []
    steps.append(('scaler', pipeline_params['scaler']))
    steps.append(('feature_selection', pipeline_params['feature_selection']))

    svm_params = {
        k.replace('svm__', ''): v
        for k, v in pipeline_params.items()
        if k.startswith('svm__')
    }
    steps.append(('svm', SVC(**svm_params)))
    svm = Pipeline(steps)
    svm.fit(train_features, train_target)

    return svm.predict(test_features)
