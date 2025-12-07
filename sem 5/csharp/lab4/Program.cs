using System.Runtime.InteropServices;
using System.Text;

class VData
{
    public enum GridType
    {
        Random,
        Geometric,
        Chebyshev,
        CentralDense
    }

    public int Segments { get; private set; }
    public double[] Xs { get; private set; }
    public double[] Ys { get; private set; }
    public double LeftDerivative { get; private set; }
    public double RightDerivative { get; private set; }
    public Func<double, double> Func { get; private set; }

    public VData(int N, double x0, double xN, Func<double, double> F, Func<double , double> DF)
    {
        Segments = N;
        Xs = GenerateGrid(x0, xN, Segments, GridType.Random);
        Ys = [.. Xs.Select(x => F(x))];
        LeftDerivative = DF(x0);
        RightDerivative = DF(xN);
        Func = F;
    }

    private static double[] GenerateGrid(double x0, double xN, int N, GridType type)
    {
        return type switch
        {
            GridType.Random => GenerateRandomGrid(x0, xN, N),
            GridType.Geometric => GenerateGeometricGrid(x0, xN, N),
            GridType.Chebyshev => GenerateChebyshevGrid(x0, xN, N),
            GridType.CentralDense => GenerateCentralDenseGrid(x0, xN, N),
            _ => GenerateRandomGrid(x0, xN, N)
        };
    }
    
    private static double[] GenerateRandomGrid(double x0, double xN, int N)
    {
        Random rand = new();
        double[] grid = new double[N + 1];
        grid[0] = x0;
        double[] intervals = new double[N];
        double total = 0;
        for (int i = 0; i < N; i++)
        {
            intervals[i] = 0.5 + rand.NextDouble(); // in [0.5,1.5]
            total += intervals[i];
        }
        double scale = (xN - x0) / total;
        for (int i = 0; i < N; i++)
            grid[i + 1] = grid[i] + intervals[i] * scale;
        grid[N] = xN;
        return grid;
    }

    private static double[] GenerateGeometricGrid(double x0, double xN, int N)
    {
        double[] grid = new double[N + 1];
        grid[0] = x0;
        double q = 1.1;
        double firstStep = (xN - x0) * (1 - q) / (1 - Math.Pow(q, N));
        for (int i = 0; i < N; i++)
        {
            double step = firstStep * Math.Pow(q, i);
            grid[i + 1] = (i == 0) ? x0 + step : grid[i] + step;
        }
        grid[N] = xN;
        return grid;
    }

    
    private static double[] GenerateChebyshevGrid(double x0, double xN, int N)
    {
        double[] grid = new double[N + 1];
        for (int i = 0; i <= N; i++)
        {
            double chebNode = Math.Cos(Math.PI * (2 * i + 1) / (2 * (N + 1)));
            grid[i] = 0.5 * (x0 + xN) + 0.5 * (xN - x0) * chebNode;
        }
        Array.Sort(grid);
        grid[0] = x0;
        grid[N] = xN;
        return grid;
    }
    
    private static double[] GenerateCentralDenseGrid(double x0, double xN, int N)
    {
        double[] grid = new double[N + 1];
        double midpoint = (x0 + xN) / 2;
        for (int i = 0; i <= N; i++)
        {
            double t = (double)i / N;
            double s = 4 * (t - 0.5) * (t - 0.5);
            if (t < 0.5) s = -s;
            grid[i] = midpoint + (xN - x0) * 0.4 * s;
        }
        Array.Sort(grid);
        grid[0] = x0;
        grid[N] = xN;
        return grid;
    }

    public override string ToString() =>
        $"VData {{ Xs=[{string.Join(", ", Xs.Select(x => $"{x:F4}"))}], " +
           $"Ys=[{string.Join(", ", Ys.Select(y => $"{y:F4}"))}], " +
           $"F'({Xs[0]:F4})={LeftDerivative:F4}, F'({Xs[Segments]:F4})={RightDerivative:F4} }}";
}

class SplineData
{
    public VData VertexData { get; private set; }
    public int SegmentsCount { get; private set; }
    public int UniformGridCount { get; private set; }

    public double InitialResidualApproximation { get; private set; }
    public int IterationsCount { get; private set; }
    public double MinResidual { get; private set; }
    public string StopReason { get; private set; }

    public double[] UniformGrid { get; private set; }
    public double[] InterpolationValues { get; private set; }
    public double[] ApproximationValues { get; private set; }

    public SplineData(VData vData, int nS, int nG)
    {
        VertexData = vData;
        SegmentsCount = nS;
        UniformGridCount = nG;
        
        // Создание равномерной сетки
        UniformGrid = new double[nG + 1];
        InterpolationValues = new double[nG + 1];
        ApproximationValues = new double[nG + 1];
        
        double x0 = vData.Xs[0];
        double xN = vData.Xs[vData.Segments];
        
        for (int i = 0; i <= nG; i++)
        {
            UniformGrid[i] = x0 + i * (xN - x0) / nG;
        }
        
        // Инициализация остальных полей
        InitialResidualApproximation = 0;
        IterationsCount = 0;
        MinResidual = 0;
        StopReason = "";

    }

    public bool Interpolate()
    {
        try
        {
            bool success = InterpolateSpline(
                VertexData.Segments + 1,      // количество точек (N+1)
                VertexData.Xs,                // массив x координат
                VertexData.Ys,                // массив y значений
                VertexData.LeftDerivative,    // производная на левом конце
                VertexData.RightDerivative,   // производная на правом конце
                UniformGridCount + 1,         // количество точек для вычисления
                UniformGrid,                  // точки для вычисления
                InterpolationValues           // результаты интерполяции
            );

            return success;
        }
        catch (Exception ex)
        {
            StopReason = $"Ошибка интерполяции: {ex.Message}";
            return false;
        }
    }

    public int Approximate(int nMaxIter, double eps, ref int resIter, ref double resMin)
    {
        try
        {
            int result = ApproximateSpline(
                VertexData.Segments + 1,      // количество исходных точек
                VertexData.Xs,                // массив x координат
                VertexData.Ys,                // массив y значений
                VertexData.LeftDerivative,    // производная на левом конце
                VertexData.RightDerivative,   // производная на правом конце
                SegmentsCount + 1,            // количество узлов сглаживающего сплайна (nS)
                UniformGridCount + 1,         // количество точек для вычисления
                nMaxIter,                     // максимальное число итераций
                eps,                          // точность (критерий остановки)
                out int iterations,           // выход: число выполненных итераций
                out double initialResidual,   // выход: начальная невязка
                out double residual,          // выход: минимальное значение невязки
                UniformGrid,                  // точки для вычисления
                ApproximationValues           // результаты аппроксимации
            );
            
            IterationsCount = iterations;
            InitialResidualApproximation = initialResidual;  // <-- сохраняем начальную невязку
            MinResidual = residual;
            resIter = iterations;
            resMin = residual;

            StopReason = GetStopReason(result, iterations, nMaxIter, residual, eps);
            
            return result;
        }
        catch (Exception ex)
        {
            StopReason = $"Ошибка аппроксимации: {ex.Message}";
            return -1;
        }
    }

    public override string ToString()
    {
    StringBuilder sb = new();
    
    // Данные VData
    sb.AppendLine("=== VData ===");
    sb.AppendLine(VertexData.ToString());
    sb.AppendLine();
    
    // Параметры сплайна
    sb.AppendLine("=== Spline Parameters ===");
    sb.AppendLine($"nS: {SegmentsCount}");
    sb.AppendLine($"nG: {UniformGridCount}");
    sb.AppendLine($"Initial Residual: {InitialResidualApproximation:E6}");
    sb.AppendLine($"Min Residual: {MinResidual:E6}");
    sb.AppendLine($"Iterations: {IterationsCount}");
    sb.AppendLine($"Stop Reason: {StopReason}");
    sb.AppendLine();
    
    // Таблица значений (с F)
    sb.AppendLine("=== Results ===");
    sb.AppendLine("Index\tX\tF(X)\tInterpolation\tApproximation");
    
    for (int i = 0; i <= UniformGridCount; i++)
    {
        // Нужна функция F - ее нужно где-то хранить
        double x = UniformGrid[i];
        double fValue = VertexData.Func(x);
        sb.AppendLine($"{i}\t{x:F6}\t{fValue:F6}\t" +
                     $"{InterpolationValues[i]:F6}\t" +
                     $"{ApproximationValues[i]:F6}");
    }
    
    return sb.ToString();
}
    
    public void Save(string filename)
    {
        try
        {
            File.WriteAllText(filename, ToString());
            Console.WriteLine($"Results saved to: {filename}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error saving file {filename}: {ex.Message}");
        }
    }

    [DllImport("libsplines.so", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool InterpolateSpline(
        int n,                    
        [In] double[] x,        
        [In] double[] y,        
        double leftDerivative,  
        double rightDerivative, 
        int nOut,                // количество точек для вычисления
        [In] double[] newPoints, // массив x координат для вычисления
        [Out] double[] splineValues // массив вычисленных значений сплайна
    );

    // Функция аппроксимации сплайна
    [DllImport("libsplines.so", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I4)]
    public static extern int ApproximateSpline(
        int n,                    // количество исходных точек
        [In] double[] x,         // массив x координат исходных точек
        [In] double[] y,         // массив y значений исходных точек
        double leftDerivative,   // производная на левом конце
        double rightDerivative,  // производная на правом конце
        int nS,                  // количество узлов сглаживающего сплайна (SegmentsCount + 1)
        int nOut,                // количество точек для вычисления
        int maxIterations,       // максимальное число итераций
        double epsilon,          // точность (критерий остановки)
        out int nIterations,     // выход: число выполненных итераций
        out double initialResidual, // выход: начальное значение невязки
        out double minResidual,  // выход: минимальное значение невязки
        [In] double[] newPoints, // массив x координат для вычисления
        [Out] double[] splineValues // массив вычисленных значений сплайна
    );

    private static string GetStopReason(int status, int iterations, int maxIterations, double residual, double epsilon)
    {
        return status switch
        {
            0 => residual < epsilon 
                ? $"Достигнута точность {epsilon:E6} за {iterations} итераций"
                : iterations >= maxIterations 
                    ? $"Достигнут лимит {maxIterations} итераций"
                    : $"Итерации остановлены (невязка {residual:E6})",
            1 => $"Достигнут предел итераций ({maxIterations})",
            2 => $"Ошибка: неверные аргументы функции",
            3 => $"Ошибка выделения памяти в C++ библиотеке",
            4 => $"Ошибка GSL библиотеки",
            5 => $"Числовая ошибка (деление на ноль, переполнение)",
            -1 => $"Ошибка аппроксимации: исключение в C#",
            -100 => $"Библиотека libsplines.so не найдена",
            -101 => $"Функция ApproximateSpline не найдена в библиотеке",
            -102 => $"Неожиданная ошибка в C#",
            _ => $"Неизвестный код ошибки: {status}"
        };
    }
}

class Program
{
    public static void Main()
{
        Console.WriteLine("=== Testing Spline Interpolation and Approximation ===");

        // Кубический многочлен: f(x) = 2x^3 - 3x^2 + 4x - 1
        static double cubicFunc(double x) => 2 * x * x * x - 3 * x * x + 4 * x - 1;
        static double cubicDerivative(double x) => 6 * x * x - 6 * x + 4;
        // Кубический многочлен: f(x) = 2x^4 - 3x^2 + 4x - 1
        // static double cubicFunc(double x) => 2 * x * x * x * x - 3 * x * x + 4 * x - 1;
        // static double cubicDerivative(double x) => 8 * x * x * x - 6 * x + 4;

        int N = 100;
        double x0 = 0.0;
        double xN = 5.0;

        VData vData = new(N, x0, xN, cubicFunc, cubicDerivative);
        Console.WriteLine("\nCreated VData:");
        Console.WriteLine(vData.ToString());

        int nS = 3;      // узлы сглаживающего сплайна
        int nG = 2 * N;  // узлы равномерной сетки

        SplineData splineData = new(vData, nS, nG);

        Console.WriteLine("\n=== Performing Interpolation ===");
        bool interpolationSuccess = splineData.Interpolate();
        
        if (interpolationSuccess)
        {
            Console.WriteLine("Interpolation completed successfully!");
            Console.WriteLine($"Initial residual: {splineData.InitialResidualApproximation:E6}");
        }
        else
        {
            Console.WriteLine($"Interpolation failed: {splineData.StopReason}");
        }

        Console.WriteLine("\n=== Performing Approximation ===");
        int maxIterations = 1000;
        double epsilon = 1e-6;
        int resIter = 0;
        double resMin = 0.0;
        
        int result = splineData.Approximate(maxIterations, epsilon, ref resIter, ref resMin);
        
        if (result == 0)
        {
            Console.WriteLine("Approximation completed successfully!");
            Console.WriteLine($"Iterations: {resIter}");
            Console.WriteLine($"Min residual: {resMin:E6}");
        }
        else
        {
            Console.WriteLine($"Approximation failed with code: {result}");
        }
        Console.WriteLine($"Stop reason: {splineData.StopReason}");

        // Сохраняем результаты
        string filename = "spline_results.txt";
        splineData.Save(filename);

        // Выводим несколько значений для проверки
        Console.WriteLine("\n=== First 5 Values ===");
        Console.WriteLine("X\t\tInterpolation\t\tApproximation");
        for (int i = 0; i < Math.Min(5, splineData.UniformGridCount + 1); i++)
        {
            Console.WriteLine($"{splineData.UniformGrid[i]:F4}\t\t" +
                            $"{splineData.InterpolationValues[i]:F6}\t\t" +
                            $"{splineData.ApproximationValues[i]:F6}");
        }
    }
}