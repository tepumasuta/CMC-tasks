/// Это я сдал, но библиотеку таки заюзал
#if 0
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

struct CubicSplineSegment {
    double a, b, c, d;
    double x0, x1;
};

class CubicSpline {
private:
    std::vector<CubicSplineSegment> segments;
    
public:
    static CubicSpline createNatural(std::vector<double> x, std::vector<double> y) {
        int n = x.size() - 1;
        if (n < 1) throw std::invalid_argument("Need at least 2 points");
        
        std::vector<double> h(n), alpha(n), l(n+1), mu(n+1), z(n+1);
        std::vector<CubicSplineSegment> segments(n);
        
        for (int i = 0; i < n; i++) {
            h[i] = x[i+1] - x[i];
        }
        
        l[0] = 1.0;
        mu[0] = 0.0;
        z[0] = 0.0;
        
        for (int i = 1; i < n; i++) {
            alpha[i] = (3.0/h[i])*(y[i+1]-y[i]) - (3.0/h[i-1])*(y[i]-y[i-1]);
            l[i] = 2.0*(x[i+1]-x[i-1]) - h[i-1]*mu[i-1];
            mu[i] = h[i]/l[i];
            z[i] = (alpha[i] - h[i-1]*z[i-1])/l[i];
        }

        l[n] = 1.0;
        z[n] = 0.0;

        for (int i = n-1; i >= 0; i--) {
            segments[i].c = z[i];
            segments[i].b = (y[i+1]-y[i])/h[i] - h[i]*(z[i+1]+2.0*z[i])/3.0;
            segments[i].d = (z[i+1]-z[i])/(3.0*h[i]);
            segments[i].a = y[i];
            segments[i].x0 = x[i];
            segments[i].x1 = x[i+1];
        }
        
        CubicSpline spline;
        spline.segments = segments;
        return spline;
    }
    
    static CubicSpline createClamped(std::vector<double> x, std::vector<double> y, 
                                     double leftDerivative, double rightDerivative) {
        int n = x.size() - 1;
        if (n < 1) throw std::invalid_argument("Need at least 2 points");
        
        std::vector<double> h(n), alpha(n+1), l(n+1), mu(n+1), z(n+1);
        std::vector<CubicSplineSegment> segments(n);
        
        for (int i = 0; i < n; i++) {
            h[i] = x[i+1] - x[i];
        }
        
        alpha[0] = 3.0*(y[1]-y[0])/h[0] - 3.0*leftDerivative;
        alpha[n] = 3.0*rightDerivative - 3.0*(y[n]-y[n-1])/h[n-1];
        
        for (int i = 1; i < n; i++) {
            alpha[i] = (3.0/h[i])*(y[i+1]-y[i]) - (3.0/h[i-1])*(y[i]-y[i-1]);
        }
        
        l[0] = 2.0*h[0];
        mu[0] = 0.5;
        z[0] = alpha[0]/l[0];
        
        for (int i = 1; i < n; i++) {
            l[i] = 2.0*(h[i]+h[i-1]) - h[i-1]*mu[i-1];
            mu[i] = h[i]/l[i];
            z[i] = (alpha[i] - h[i-1]*z[i-1])/l[i];
        }
        
        l[n] = h[n-1]*(2.0 - mu[n-1]);
        z[n] = (alpha[n] - h[n-1]*z[n-1])/l[n];
        
        std::vector<double> c(n+1);
        c[n] = z[n];
        for (int i = n-1; i >= 0; i--) {
            c[i] = z[i] - mu[i]*c[i+1];
        }
        
        for (int i = 0; i < n; i++) {
            segments[i].a = y[i];
            segments[i].c = c[i];
            segments[i].b = (y[i+1]-y[i])/h[i] - h[i]*(c[i+1]+2.0*c[i])/3.0;
            segments[i].d = (c[i+1]-c[i])/(3.0*h[i]);
            segments[i].x0 = x[i];
            segments[i].x1 = x[i+1];
        }
        
        CubicSpline spline;
        spline.segments = segments;
        return spline;
    }
    
    double evaluate(double x) const {
        for (const auto& seg : segments) {
            if (x >= seg.x0 - 1e-10 && x <= seg.x1 + 1e-10) {
                double dx = x - seg.x0;
                return seg.a + seg.b*dx + seg.c*dx*dx + seg.d*dx*dx*dx;
            }
        }
        
        if (x < segments.front().x0) {
            const auto& seg = segments.front();
            double dx = x - seg.x0;
            return seg.a + seg.b*dx + seg.c*dx*dx + seg.d*dx*dx*dx;
        } else {
            const auto& seg = segments.back();
            double dx = x - seg.x0;
            return seg.a + seg.b*dx + seg.c*dx*dx + seg.d*dx*dx*dx;
        }
    }
    
    double derivative(double x) const {
        for (const auto& seg : segments) {
            if (x >= seg.x0 - 1e-10 && x <= seg.x1 + 1e-10) {
                double dx = x - seg.x0;
                return seg.b + 2.0*seg.c*dx + 3.0*seg.d*dx*dx;
            }
        }

        if (x < segments.front().x0) {
            const auto& seg = segments.front();
            double dx = x - seg.x0;
            return seg.b + 2.0*seg.c*dx + 3.0*seg.d*dx*dx;
        } else {
            const auto& seg = segments.back();
            double dx = x - seg.x0;
            return seg.b + 2.0*seg.c*dx + 3.0*seg.d*dx*dx;
        }
    }
};

double calculateResidual(const std::vector<double>& x, const std::vector<double>& y,
                         const CubicSpline& spline) {
    double residual = 0.0;
    for (size_t i = 0; i < x.size(); i++) {
        double diff = spline.evaluate(x[i]) - y[i];
        residual += diff * diff;
    }
    return residual;
}

class GradientDescentOptimizer {
private:
    double learningRate;
    int maxIterations;
    double epsilon;
    
public:
    GradientDescentOptimizer(double lr = 0.01, int maxIter = 1000, double eps = 1e-6)
        : learningRate(lr), maxIterations(maxIter), epsilon(eps) {}
    
    // Оптимизация значений сплайна в узлах
    bool optimize(std::vector<double>& zValues, const std::vector<double>& x,
                  const std::vector<double>& y, const std::vector<double>& zGrid,
                  double leftDerivative, double rightDerivative,
                  int& iterations, double& minResidual) {
        
        double prevResidual = std::numeric_limits<double>::max();
        
        for (iterations = 0; iterations < maxIterations; iterations++) {
            // Строим сплайн по текущим значениям
            CubicSpline spline = CubicSpline::createClamped(zGrid, zValues, 
                                                            leftDerivative, rightDerivative);
            
            // Вычисляем невязку
            double residual = calculateResidual(x, y, spline);
            minResidual = residual;
            
            // Проверка сходимости
            if (std::abs(prevResidual - residual) < epsilon || residual < epsilon) {
                return true; // Сходимость достигнута
            }
            
            // Простейший градиентный спуск (упрощенная версия)
            // В реальной реализации нужно вычислять градиент аналитически или через конечные разности
            for (size_t i = 1; i < zValues.size() - 1; i++) {
                // Упрощенное обновление: двигаемся в сторону уменьшения ошибки
                double step = learningRate * (y[i % y.size()] - spline.evaluate(x[i % x.size()]));
                zValues[i] += step;
            }
            
            prevResidual = residual;
        }
        
        return false; // Достигнуто максимальное число итераций
    }
};

// Интерполяционный сплайн
extern "C"  bool InterpolateSpline(
    int n, double x[], double y[], 
    double leftDerivative, double rightDerivative,
    int nOut, double newPoints[], double splineValues[]) {
    
    try {
        std::vector<double> xVec(x, x + n);
        std::vector<double> yVec(y, y + n);
        
        // Проверка входных данных
        if (xVec.size() < 2) return false;
        
        // Построение сплайна с заданными производными на концах
        CubicSpline spline = CubicSpline::createClamped(xVec, yVec, 
                                                        leftDerivative, rightDerivative);
        
        // Вычисление значений в новых точках
        for (int i = 0; i < nOut; i++) {
            splineValues[i] = spline.evaluate(newPoints[i]);
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// Функция аппроксимации сплайна
extern "C"  int ApproximateSpline(
    int n, double x[], double y[], 
    double leftDerivative, double rightDerivative,
    int nS, int nOut, int maxIterations, double epsilon,
    int* nIterations, double* minResidual,
    double newPoints[], double splineValues[]) {
    
    try {
        std::vector<double> xVec(x, x + n);
        std::vector<double> yVec(y, y + n);
        
        if (xVec.size() < 2 || nS < 1) return -1;
        
        // Создание равномерной сетки для узлов сглаживающего сплайна
        double x0 = xVec.front();
        double xN = xVec.back();
        std::vector<double> zGrid(nS + 1);
        
        for (int i = 0; i <= nS; i++) {
            zGrid[i] = x0 + i * (xN - x0) / nS;
        }
        
        // Начальное приближение: строим интерполяционный сплайн по исходным данным
        // и вычисляем его значения в узлах zGrid
        CubicSpline initialSpline = CubicSpline::createClamped(xVec, yVec, 
                                                               leftDerivative, rightDerivative);
        std::vector<double> zValues(nS + 1);
        for (int i = 0; i <= nS; i++) {
            zValues[i] = initialSpline.evaluate(zGrid[i]);
        }
        
        // Оптимизация методом градиентного спуска
        GradientDescentOptimizer optimizer(0.01, maxIterations, epsilon);
        int iterations;
        double residual;
        
        bool success = optimizer.optimize(zValues, xVec, yVec, zGrid,
                                          leftDerivative, rightDerivative,
                                          iterations, residual);
        
        // Сохранение результатов
        *nIterations = iterations;
        *minResidual = residual;
        
        // Построение финального сплайна и вычисление значений
        CubicSpline finalSpline = CubicSpline::createClamped(zGrid, zValues,
                                                             leftDerivative, rightDerivative);
        
        for (int i = 0; i < nOut; i++) {
            splineValues[i] = finalSpline.evaluate(newPoints[i]);
        }
        
        return success ? 0 : 1; // 0 - успех, 1 - достигнуто максимальное число итераций
        
    } catch (const std::exception& e) {
        return -1; // Ошибка
    }
}

#endif












#define _GLIBCXX_DEBUG 1


#include "splines.hpp"

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include "spline.h"

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_errno.h>

namespace
{
    double constexpr CLAMP01(double t)
    {
        return t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
    }

    enum ERROR_CODE : int
    {
        ERR_NONE                = 0,
        ERR_MAX_ITER_REACHED    = 1,
        ERR_BAD_ARGS            = 2,
        ERR_ALLOC               = 3,
        ERR_GSL                 = 4,
        ERR_NUMERIC             = 5
    };

    struct GSLMatrix
    {
        gsl_matrix* ptr;
        
        GSLMatrix(size_t n1, size_t n2) : ptr(gsl_matrix_alloc(n1, n2)) {}
        ~GSLMatrix() { if (ptr) gsl_matrix_free(ptr); }
        
        double& operator()(size_t i, size_t j) { return *gsl_matrix_ptr(ptr, i, j); }
        double operator()(size_t i, size_t j) const { return gsl_matrix_get(ptr, i, j); }
        
        operator bool() const { return ptr != nullptr; }
    };

    struct GSLVector
    {
        gsl_vector* ptr;
        
        GSLVector(size_t n) : ptr(n > 0 ? gsl_vector_alloc(n) : nullptr) {}
        ~GSLVector() { if (ptr) gsl_vector_free(ptr); }
        
        double& operator[](size_t i) { return *gsl_vector_ptr(ptr, i); }
        double operator[](size_t i) const { return gsl_vector_get(ptr, i); }
        
        void ZERO() { if (ptr) gsl_vector_set_zero(ptr); }
        void MEMCPY(const GSLVector& other) { 
            if (ptr && other.ptr) 
                gsl_vector_memcpy(ptr, other.ptr); 
        }
        
        operator bool() const { return ptr != nullptr; }
        size_t size() const { return ptr ? ptr->size : 0; }
    };

    struct GSLAutoHandler
    {
        gsl_error_handler_t* old;
        GSLAutoHandler() : old(gsl_set_error_handler_off()) {}
        ~GSLAutoHandler() { gsl_set_error_handler(old); }
    };

    template<typename T>
    T* SAFE_ALLOC(T* ptr, const char* msg)
    {
        if (!ptr)
            std::cerr << "[SplineApproximate] ALLOC FAILED: " << msg << "\n";
        return ptr;
    }

    #define GSL_SAFE_ALLOC(type, ...) SAFE_ALLOC(type(__VA_ARGS__), #type)

    void GSL_DGEMV(const GSLMatrix& A, const GSLVector& x, GSLVector& y, bool trans = false)
    {
        gsl_blas_dgemv(trans ? CblasTrans : CblasNoTrans, 1.0, A.ptr, x.ptr, 0.0, y.ptr);
    }

    void GSL_DAXPY(double alpha, const GSLVector& x, GSLVector& y)
    {
        gsl_blas_daxpy(alpha, x.ptr, y.ptr);
    }

    double GSL_DDOT(const GSLVector& a, const GSLVector& b)
    {
        double result;
        gsl_blas_ddot(a.ptr, b.ptr, &result);
        return result;
    }
}

bool InterpolateSpline(
    int nPoints,
    double* x,
    double* y,
    double dLeft, 
    double dRight,
    int nUniformGrid,
    double* UniformGrid,
    double* sOut)
{
    try
    {
        std::vector<double> vx(x, x + nPoints);
        std::vector<double> vy(y, y + nPoints);
        
        tk::spline spline;
        spline.set_boundary(
            tk::spline::first_deriv, dLeft,
            tk::spline::first_deriv, dRight
        );
        spline.set_points(vx, vy);
        
        for (int i = 0; i < nUniformGrid; i++)
            sOut[i] = spline(UniformGrid[i]);
        
        return true;
    }
    catch(...)
    {
        return false;
    }       
}

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
    double* sSmoothOut)
{
    std::cerr << "=== ApproximateSpline called ===" << std::endl;
    std::cerr << "nPoints: " << nPoints << std::endl;
    std::cerr << "nS: " << nS << std::endl;
    std::cerr << "nUniformGrid: " << nUniformGrid << std::endl;
    std::cerr << "nMaxIter: " << nMaxIter << std::endl;
    std::cerr << "epsilon: " << epsilon << std::endl;
    std::cerr.flush();

    // Проверки входных данных
    if (!x || !y || !UniformGrid || !outIter || !outRes0 || !outResMin || !sSmoothOut) {
        std::cerr << "ERROR: Null pointer argument!" << std::endl;
        return ERR_BAD_ARGS;
    }
    
    if (nPoints < 2 || nS < 2 || nUniformGrid < 1 || nMaxIter < 1 || epsilon <= 0.0) {
        std::cerr << "ERROR: Invalid arguments: nPoints=" << nPoints 
                  << " nS=" << nS << " nUniformGrid=" << nUniformGrid
                  << " nMaxIter=" << nMaxIter << " epsilon=" << epsilon << std::endl;
        return ERR_BAD_ARGS;
    }

    // Количество узлов сглаживающего сплайна = nS (передается из C# как SegmentsCount + 1)
    // Количество интервалов = nS - 1
    const int M = nS;                    // количество узлов сглаживающего сплайна
    const int N = M - 1;                 // количество интервалов
    const int KU = (N >= 2) ? (N - 1) : 0;  // внутренние производные

    const double X0 = x[0];
    const double XN = x[nPoints - 1];
    
    if (!(XN > X0))
        return ERR_BAD_ARGS;

    // Шаг равномерной сетки для сглаживающего сплайна
    const double HZ = (XN - X0) / static_cast<double>(N);
    if (!(HZ > 0.0) || !std::isfinite(HZ))
        return ERR_NUMERIC;

    GSLAutoHandler gsl_guard;

    // Выделение памяти для GSL структур
    GSLMatrix A(nPoints, M);
    GSLVector bvals(nPoints);
    GSLVector ytilde(nPoints);
    GSLVector rN(nPoints);
    GSLVector tmpN(nPoints);
    GSLVector s0(M);
    GSLVector sCur(M);
    GSLVector sBest(M);
    GSLVector grad(M);
    GSLVector kAll(M);

    GSLVector tri_a(KU >= 2 ? KU - 1 : 0);
    GSLVector tri_b(KU >= 1 ? KU : 0);
    GSLVector tri_c(KU >= 2 ? KU - 1 : 0);
    GSLVector tri_f(KU >= 1 ? KU : 0);
    GSLVector tri_x(KU >= 1 ? KU : 0);

    GSLMatrix M2(KU == 2 ? 2 : 0, KU == 2 ? 2 : 0);
    GSLVector b2(KU == 2 ? 2 : 0);
    GSLVector x2(KU == 2 ? 2 : 0);
    gsl_permutation* P2 = (KU == 2) ? gsl_permutation_alloc(2) : nullptr;

    // Проверка успешности выделения памяти
    if (!A || !bvals || !ytilde || !rN || !tmpN || !s0 || !sCur || !sBest || !grad || !kAll ||
        (KU >= 1 && (!tri_b || !tri_f || !tri_x)) ||
        (KU >= 2 && (!tri_a || !tri_c)) ||
        (KU == 2 && (!M2 || !b2 || !x2 || !P2)))
    {
        if (P2) gsl_permutation_free(P2);
        return ERR_ALLOC;
    }

    // Функция для построения коэффициентов k (производных в узлах)
    auto BUILD_K = [&](const GSLVector& s_vals) -> int
    {
        kAll[0] = dLeft;
        kAll[N] = dRight;
        
        if (KU == 0)
            return GSL_SUCCESS;

        if (tri_a) tri_a.ZERO();
        if (tri_c) tri_c.ZERO();
        tri_b.ZERO();
        tri_f.ZERO();

        // Уравнения для внутренних производных
        for (int i = 1; i <= N - 1; ++i)
        {
            const int u = i - 1;
            const double si_m1 = s_vals[i - 1];
            const double si_p1 = s_vals[i + 1];
            double rhs_i = 3.0 * (si_p1 - si_m1);

            tri_b[u] = 4.0 * HZ;
            tri_f[u] = rhs_i;

            if (i == 1)
                tri_f[u] -= HZ * dLeft;
            else
                tri_a[u - 1] = HZ;

            if (i == N - 1)
                tri_f[u] -= HZ * dRight;
            else
                tri_c[u] = HZ;
        }

        int status = GSL_SUCCESS;

        // Решение тридиагональной системы
        if (KU == 1)
        {
            const double denom = tri_b[0];
            if (!std::isfinite(denom) || denom == 0.0)
                return GSL_EZERODIV;
            tri_x[0] = tri_f[0] / denom;
        }
        else if (KU == 2)
        {
            M2(0, 0) = tri_b[0];
            M2(0, 1) = tri_c[0];
            M2(1, 0) = tri_a[0];
            M2(1, 1) = tri_b[1];
            b2[0] = tri_f[0];
            b2[1] = tri_f[1];

            int signum = 0;
            status = gsl_linalg_LU_decomp(M2.ptr, P2, &signum);
            if (status != GSL_SUCCESS)
                return status;
            status = gsl_linalg_LU_solve(M2.ptr, P2, b2.ptr, x2.ptr);
            if (status != GSL_SUCCESS)
                return status;

            tri_x[0] = x2[0];
            tri_x[1] = x2[1];
        }
        else
        {
            status = gsl_linalg_solve_tridiag(tri_a.ptr, tri_b.ptr, tri_c.ptr, tri_f.ptr, tri_x.ptr);
            if (status != GSL_SUCCESS)
                return status;
        }

        for (int u = 0; u < KU; ++u)
            kAll[u + 1] = tri_x[u];

        return GSL_SUCCESS;
    };

    // Линейная интерполяция для начального приближения
    auto INTERP_Y_AT = [&](double xx) -> double
    {
        if (xx <= x[0])
            return y[0];
        if (xx >= x[nPoints - 1])
            return y[nPoints - 1];

        int lo = 0, hi = nPoints - 1;
        while (hi - lo > 1)
        {
            int mid = (lo + hi) / 2;
            if (xx < x[mid])
                hi = mid;
            else
                lo = mid;
        }

        const double xL = x[lo], xR = x[hi];
        const double yL = y[lo], yR = y[hi];
        const double t = (xx - xL) / (xR - xL);
        
        return yL + t * (yR - yL);
    };

    // Вычисление сплайна в заданных точках
    auto EVAL_SPLINE = [&](const GSLVector& s_vals, const double* X,
                          int n_eval, double* out_s) -> int
    {
        const int status = BUILD_K(s_vals);
        if (status != GSL_SUCCESS)
            return status;

        for (int t = 0; t < n_eval; ++t)
        {
            double xx = X[t];
            if (xx <= X0) xx = X0;
            if (xx >= XN) xx = XN;

            int i = static_cast<int>(std::floor((xx - X0) / HZ));
            if (i < 0) i = 0;
            if (i > N - 1) i = N - 1;

            const double zi = X0 + static_cast<double>(i) * HZ;
            const double tau = CLAMP01((xx - zi) / HZ);

            const double s_i   = s_vals[i];
            const double s_ip1 = s_vals[i + 1];
            const double k_i   = kAll[i];
            const double k_ip1 = kAll[i + 1];

            const double t2 = tau * tau;
            const double t3 = t2 * tau;

            // Эрмитовы базисные функции
            const double h00 =  2.0 * t3 - 3.0 * t2 + 1.0;
            const double h10 =  t3 - 2.0 * t2 + tau;
            const double h01 = -2.0 * t3 + 3.0 * t2;
            const double h11 =  t3 - t2;

            out_s[t] = h00 * s_i + h10 * (HZ * k_i) + h01 * s_ip1 + h11 * (HZ * k_ip1);
        }

        return GSL_SUCCESS;
    };

    // Вычисление базисных функций (для матрицы A)
    sCur.ZERO();
    {
        double* buf = gsl_vector_ptr(tmpN.ptr, 0);
        const int status = EVAL_SPLINE(sCur, x, nPoints, buf);
        if (status != GSL_SUCCESS)
        {
            if (P2) gsl_permutation_free(P2);
            return ERR_GSL;
        }
        
        for (int i = 0; i < nPoints; ++i)
            bvals[i] = buf[i];
    }

    // Преобразование y для минимизации
    for (int i = 0; i < nPoints; ++i)
        ytilde[i] = y[i] - bvals[i];

    // Построение матрицы A (влияние каждого узла сплайна на каждую точку данных)
    for (int j = 0; j < M; ++j)
    {
        sCur.ZERO();
        sCur[j] = 1.0;

        double* buf = gsl_vector_ptr(tmpN.ptr, 0);
        const int status = EVAL_SPLINE(sCur, x, nPoints, buf);
        if (status != GSL_SUCCESS)
        {
            if (P2) gsl_permutation_free(P2);
            return ERR_GSL;
        }

        for (int i = 0; i < nPoints; ++i)
            A(i, j) = buf[i] - bvals[i];
    }

    // Начальное приближение: линейная интерполяция в узлах сплайна
    for (int j = 0; j < M; ++j)
    {
        const double zj = X0 + static_cast<double>(j) * HZ;
        s0[j] = INTERP_Y_AT(zj);
    }
    
    sCur.MEMCPY(s0);
    sBest.MEMCPY(s0);

    // Вычисление начальной невязки
    auto COMPUTE_RES = [&](const GSLVector& s_vals) -> double
    {
        GSL_DGEMV(A, s_vals, rN, false);
        for (int i = 0; i < nPoints; ++i)
            rN[i] -= ytilde[i];
        return GSL_DDOT(rN, rN);
    };

    double initial_res = COMPUTE_RES(sCur);
    *outRes0 = initial_res;

    // Вычисление шага градиентного спуска
    double fro2 = 0.0;
    for (int i = 0; i < nPoints; ++i)
        for (int j = 0; j < M; ++j)
            fro2 += A(i, j) * A(i, j);
    
    if (!(fro2 > 0.0) || !std::isfinite(fro2))
    {
        if (P2) gsl_permutation_free(P2);
        return ERR_NUMERIC;
    }
    
    const double ALPHA = 1.0 / fro2;  // шаг градиентного спуска

    // Итерационный процесс минимизации
    ERROR_CODE stop_code = ERR_MAX_ITER_REACHED;
    int iter_done = 0;
    double best_res = initial_res;
    bool numeric_abort = false;

    for (int iter = 0; iter <= nMaxIter; ++iter)
    {
        iter_done = iter;
        const double res = COMPUTE_RES(sCur);

        if (!std::isfinite(res))
        {
            numeric_abort = true;
            break;
        }

        if (res < best_res)
        {
            best_res = res;
            sBest.MEMCPY(sCur);
        }

        // Проверка критерия остановки
        if (best_res <= epsilon)
        {
            stop_code = ERR_NONE;
            break;
        }
        
        if (iter == nMaxIter)
            break;

        // Градиентный шаг
        GSL_DGEMV(A, rN, grad, true);
        GSL_DAXPY(-ALPHA, grad, sCur);

        // Проверка числовой стабильности
        for (int j = 0; j < M; ++j)
            if (!std::isfinite(sCur[j]))
            {
                numeric_abort = true;
                iter_done = iter + 1;
                break;
            }
        
        if (numeric_abort)
            break;
    }

    *outIter = iter_done;
    *outResMin = best_res;

    // Вычисление сплайна на равномерной сетке
    {
        const int status = EVAL_SPLINE(sBest, UniformGrid, nUniformGrid, sSmoothOut);
        if (status != GSL_SUCCESS)
        {
            if (P2) gsl_permutation_free(P2);
            return ERR_GSL;
        }
    }

    if (P2) gsl_permutation_free(P2);

    if (numeric_abort)
        return ERR_NUMERIC;

    std::cerr << "Returning error code: " << stop_code << std::endl;

    return static_cast<int>(stop_code);
}