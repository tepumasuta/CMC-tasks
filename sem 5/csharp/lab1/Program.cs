using System.Numerics;
using System.Text;

struct DataItemF(float x, float y, Vector2 v)
{
    public float X = x, Y = y;
    public Vector2 Data = v;

    public override readonly string ToString()
        => $"DataItemF {{x={X},y={Y},data={Data}}}";
}

class V1DataArray
{
    private readonly float[] _packedData;

    public string Key { get; set; }
    public (int, float) Xgrid { get; private set; }
    public (int, float) Ygrid { get; private set; }
    public int Count { get => Xgrid.Item1 * Ygrid.Item1; }
    public (float, float) MinMax { get => (_packedData.Min(), _packedData.Max()); }
    public DataItemF? this[int x, int y]
    {
        get => x >= Xgrid.Item1 || y >= Ygrid.Item1 || x < 0 || y < 0
            ? null
            : new DataItemF(x, y, new (
                _packedData[2 * x + 2 * y * Ygrid.Item1],
                _packedData[2 * x + 2 * y * Ygrid.Item1 + 1]
            ));
    }

    public V1DataArray(string key, (int, float) xGrid, (int, float) yGrid)
    {
        Key = key;
        Xgrid = xGrid;
        Ygrid = yGrid;

        float Min = -100.0F, Max = 100.0F;
        Random randNum = new();
        _packedData = [
            .. Enumerable
            .Repeat(0, Count * 2)
            .Select(i => randNum.NextSingle() * (Max - Min) + Min)
        ];
        // Мог бы = new и цикл, но collection expression со spread читаемей
    }

    public override string ToString() =>
        $"V1DataArray {{Key={Key},Xgrid={Xgrid},Ygrid={Ygrid}}}";

    public string ToLongString() =>
        $"V1DataArray {{Key={Key},Xgrid={Xgrid},Ygrid={Ygrid},"
        + $"data=[{string.Join(',', _packedData.Select(x => string.Format("{0:0.00}", x)))}]}}";

    public static explicit operator V2RDataArray(V1DataArray arr)
    {
        V2RDataArray res = new(
            arr.Key,
            (arr.Xgrid.Item1, arr.Xgrid.Item2,
            arr.Ygrid.Item1, arr.Ygrid.Item2)
        );
        for (int i = 0; i < arr.Xgrid.Item1; i++)
            for (int j = 0; j < arr.Ygrid.Item1; j++)
            {
                var v = arr[i, j].Value.Data;
                res[i, j] = (v.X, v.Y);
            }
        return res;
    }
}

class V2RDataArray
{
    private double[,] _packedData;

    public string Key { get; set; }
    public (int, double, int, double) XYgrid { get; private set; }
    public int Count { get => XYgrid.Item1 * XYgrid.Item3; }
    public (double, int, int) Max
    {
        get {
            var (idx, max) = _packedData.Cast<double>().Index().MaxBy(x => x.Item);
            idx /= 2;
            return (max, idx % _packedData.GetLength(1), idx / _packedData.GetLength(1));
        }
    }
    public (double, double) this[int x, int y]
    {
        get => x >= XYgrid.Item1 || y >= XYgrid.Item3 || x < 0 || y < 0
            ? (double.NaN, double.NaN)
            : (_packedData[x, y], _packedData[x + 1, y]); // Ибо а как хранить по 2 double в одном?
        set
        {
            if (x < XYgrid.Item1 && y < XYgrid.Item3 && x >= 0 && y >= 0)
                (_packedData[x, y], _packedData[x + 1, y]) = value;
        }
    }

    public V2RDataArray(string key, (int, double, int, double) xyGrid)
    {
        Key = key;
        XYgrid = xyGrid;

        double Min = -100.0, Max = 100.0;
        Random randNum = new();
        _packedData = new double[XYgrid.Item1 * 2, XYgrid.Item3];
        foreach (var i in Enumerable.Range(0, _packedData.GetLength(0)))
            foreach (var j in Enumerable.Range(0, _packedData.GetLength(1)))
                _packedData[i, j] = randNum.NextDouble() * (Max - Min) + Min;
        // Мог бы = new и цикл, но collection expression со spread читаемей
    }

    public override string ToString()
    {
        return $"V2RDataArray {{Key={Key},XYgrid={XYgrid}}}";
    }

    public string ToLongString()
    {
        StringBuilder prettyArray = new();
        for (int i = 0; i < _packedData.GetLength(0); i++)
        {
            prettyArray.Append('[');
            for (int j = 0; j < _packedData.GetLength(1); j++)
            {
                prettyArray.AppendFormat("{0:0.00},", _packedData[i, j]);
            }
            prettyArray.Append("],");
        }

        return $"V2RDataArray {{Key={Key},XYgrid={XYgrid},data=[{prettyArray.ToString()}]}}";
    }
}

internal class Program
{
    public static void Main()
    {
        // Fuck this bullshit
        Thread.CurrentThread.CurrentCulture = System.Globalization.CultureInfo.CreateSpecificCulture("en-US");

        var df = new DataItemF(69, 420, new(666, -1337));
        Console.WriteLine(df.ToString());
        var d1 = new V1DataArray("БубубуБебебе", (3, 0.5F), (2, 1.3F));
        Console.WriteLine(d1.ToLongString());
        Console.WriteLine($"Count: {d1.Count}, MinMax: {d1.MinMax}");
        Console.WriteLine($"Bound: {d1[2, 0]}, Unbound: {d1[69, -69]}"); // null interpolates poorly
        var d2 = (V2RDataArray)d1;
        Console.WriteLine(d2.ToLongString());
        Console.WriteLine($"Count: {d2.Count}, Max: {d2.Max}");
        Console.WriteLine($"Bound: {d2[1, 1]}, Unbound: {d2[-69, 420]}"); // null interpolates poorly
        d2[1, 1] = (42, 69);
        Console.WriteLine(d2.ToLongString());
    }
}