#pragma once

extern "C"
{
    bool InterpolateSpline(
        int nPoints,
        double* x,
        double* y,
        double dLeft, 
        double dRight,
        int nUniformGrid,
        double* UniformGrid,
        double* sOut
    );

    int ApproximateSpline(
        int nPoints,
        double* x,
        double* y,
        double dLeft, 
        double dRight,
        int nS,
        int nUniformGrid,
        int nMaxIter,
        double epsilon,
        int* outIter,
        double* outRes0,
        double* outResMin,
        double* UniformGrid,
        double* sSmoothOut
    );
}