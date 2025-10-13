using System.Numerics;
using System.Text;

struct DataItemF(float x, float y, Vector2 v)
{
    public float X = x, Y = y;
    public Vector2 Data = v;

    public override readonly string ToString()
        => $"DataItemF {{x={X},y={Y},data={Data}}}";
}

static class Funcs
{
    public static Vector2 RotatedVecAtPoint(float x, float y) =>
        new(y, -x);

    public static DataItemF DataItemFAtIndex(int idx) =>
        new(idx, -idx, new Vector2(-idx, idx));

    public static ((float X, float Y), DataItemF Data) KeyDataItemFPairAtIndex(int idx) =>
        new((-idx * idx, idx * idx), new DataItemF(-idx * idx, idx * idx, new(-idx, idx)));
};

interface IDataInfo
{
    string Key { get; set; }
    int Count { get; }
    (float Min, float Max) MinMax { get; }
    DataItemF Nearest(float x, float y);
}

class V2RDataArray : IDataInfo, IEnumerable<DataItemF>
{
    protected readonly Vector2[,] PackedData;

    public string Key { get; set; }
    public (int size, float step) Xgrid { get; private set; }
    public (int size, float step) Ygrid { get; private set; }
    public int Count { get => Xgrid.size * Ygrid.size; }
    public (float Min, float Max) MinMax
    {
        get => (
            PackedData.Cast<Vector2>().Min(v => v.X),
            PackedData.Cast<Vector2>().Max(v => v.X)
        );
    }

    public V2RDataArray(string key, (int size, float step) Xgrid, (int size, float step) Ygrid, Func<float, float, Vector2> FR)
    {
        Key = key;
        this.Xgrid = Xgrid;
        this.Ygrid = Ygrid;

        PackedData = new Vector2[Xgrid.size, Ygrid.size];
        foreach (var i in Enumerable.Range(0, Xgrid.size))
            foreach (var j in Enumerable.Range(0, Ygrid.size))
                PackedData[i, j] = FR(i * Xgrid.step, j * Ygrid.step);
    }

    public DataItemF Nearest(float x, float y)
    {
        float x_mapped = x / Xgrid.step;
        float y_mapped = y / Ygrid.step;
        float x_fr = x_mapped - MathF.Truncate(x_mapped);
        float y_fr = y_mapped - MathF.Truncate(y_mapped);
        int x_delta = (int)MathF.Truncate(x_fr * 2);
        int y_delta = (int)MathF.Truncate(y_fr * 2);
        int x_idx = (int)Math.Truncate(x_mapped) + x_delta;
        int y_idx = (int)Math.Truncate(y_mapped) + y_delta;
        int x_clamped = Math.Clamp(x_idx, 0, Xgrid.size - 1);
        int y_clamped = Math.Clamp(y_idx, 0, Ygrid.size - 1);
        return new DataItemF(x_clamped * Xgrid.step, y_clamped * Ygrid.step, PackedData[x_clamped, y_clamped]);
    }

    System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
    {
        return GetEnumerator();
    }

    public IEnumerator<DataItemF> GetEnumerator()
    {
        for (int i = 0; i < Xgrid.size; i++)
        {
            for (int j = 0; j < Ygrid.size; j++)
                yield return new DataItemF(i * Xgrid.step, j * Ygrid.step, PackedData[i, j]);
        }
    }

    public override string ToString() =>
        $"V2DataArray {{Key={Key},Xgrid={Xgrid},Ygrid={Ygrid}}}";

    public string GridToString()
    {
        var result = new StringBuilder("data=[");
        for (int i = 0; i < Xgrid.size; i++)
        {
            for (int j = 0; j < Ygrid.size; j++)
                result.Append($"(idx=({i},{j}),coord=({i * Xgrid.step},{j * Ygrid.step}),vec={PackedData[i, j]}),");
        }
        result.Append(']');
        return result.ToString();
    }

    public virtual string ToLongString() =>
        $"V2DataArray {{Key={Key},Xgrid={Xgrid},Ygrid={Ygrid},data={GridToString()}";
}

class V2RList : V2RDataArray, IDataInfo, IEnumerable<DataItemF>
{
    private readonly List<DataItemF> _additional;
    public new string Key { get; set; }
    public new int Count { get => _additional.Count + base.Count; }
    public new (float Min, float Max) MinMax
    {
        get => (
            MathF.Min(PackedData.Cast<Vector2>().Min(v => v.X), _additional.Min(v => v.Data.X)),
            MathF.Max(PackedData.Cast<Vector2>().Max(v => v.X), _additional.Max(v => v.Data.X))
        );
    }
    public new DataItemF Nearest(float x, float y)
    {
        var minList = _additional.MinBy(d => Math.Sqrt((d.X - x) * (d.X - x) + (d.Y - y) * (d.Y - y)));
        var minGrid = base.Nearest(x, y);
        return Math.Sqrt((minList.X - x) * (minList.X - x) + (minList.Y - y) * (minList.Y - y))
               > Math.Sqrt((minGrid.X - x) * (minGrid.X - x) + (minGrid.Y - y) * (minGrid.Y - y))
            ? minGrid
            : minList;
    }

    System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
    {
        return GetEnumerator();
    }

    public new IEnumerator<DataItemF> GetEnumerator()
    {
        var enumerator = base.GetEnumerator();
        while (enumerator.MoveNext())
        {
            yield return enumerator.Current;
        }
        foreach (var d in _additional)
            yield return d;
    }

    public V2RList(string key, (int, float) Xgrid, (int, float) Ygrid,
                   Func<float, float, Vector2> FR, int nList,
                   Func<int, DataItemF> FL)
        : base(key, Xgrid, Ygrid, FR)
    {
        Key = key;
        _additional = [
            .. Enumerable
            .Range(0, nList)
            .Select(i => FL(i))
        ];
    }

    public void Add(DataItemF item)
    {
        _additional.Add(item);
    }

    public override string ToString() =>
        $"V2RList {{Key={Key},Xgrid={Xgrid},Ygrid={Ygrid},Length={_additional.Count}}}";

    public override string ToLongString() =>
        $"V2RList {{Key={Key},Xgrid={Xgrid},Ygrid={Ygrid},Length={_additional.Count}}}"
        + $",data={GridToString()},additional=[{string.Join(',', _additional)}]}}";
}

class V2RDict : IDataInfo, IEnumerable<DataItemF>
{
    private V2RDataArray _grid;
    private Dictionary<(float X, float Y), DataItemF> _additional;

    public string Key { get; set; }
    public int Count { get => _additional.Count + _grid.Count; }
    public (float Min, float Max) MinMax
    {
        get
        {
            var (baseMin, baseMax) = _grid.MinMax;
            var curMin = _additional.Values.Min(v => v.Data.X);
            var curMax = _additional.Values.Max(v => v.Data.X);
            return (MathF.Min(curMin, baseMin), MathF.Max(curMax, baseMax));
        }
    }
    public DataItemF Nearest(float x, float y)
    {
        var minList = _additional.MinBy(p => Math.Sqrt((p.Key.X - x) * (p.Key.X - x) + (p.Key.Y - y) * (p.Key.Y - y))).Value;
        var minGrid = _grid.Nearest(x, y);
        return Math.Sqrt((minList.X - x) * (minList.X - x) + (minList.Y - y) * (minList.Y - y))
               > Math.Sqrt((minGrid.X - x) * (minGrid.X - x) + (minGrid.Y - y) * (minGrid.Y - y))
            ? minGrid
            : minList;
    }

    System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
    {
        return GetEnumerator();
    }

    public IEnumerator<DataItemF> GetEnumerator()
    {
        var enumerator = _grid.GetEnumerator();
        while (enumerator.MoveNext())
        {
            yield return enumerator.Current;
        }
        foreach (var (_, d) in _additional)
            yield return d;
    }


    public V2RDict(string key, (int, float) Xgrid, (int, float) Ygrid, Func<float, float, Vector2> FR,
            int nDict, Func<int, ((float, float), DataItemF)> FD)
    {
        Key = key;
        _grid = new V2RDataArray(key, Xgrid, Ygrid, FR);
        _additional = new Dictionary<(float X, float Y), DataItemF>(nDict);
        foreach (var i in Enumerable.Range(0, nDict))
        {
            var (k, v) = FD(i);
            _additional[k] = v;
        }
    }

    public void Add(DataItemF item)
    {
        _additional.Add((item.X, item.Y), item);
    }

    public override string ToString() =>
        $"V2RDict {{Key={Key},Xgrid={_grid.Xgrid},Ygrid={_grid.Ygrid},Length={_additional.Count}}}";

    public string ToLongString() =>
        $"V2RDict {{Key={Key},Xgrid={_grid.Xgrid},Ygrid={_grid.Ygrid},Length={_additional.Count}}}"
        + $",data={_grid.GridToString()},additional=[{string.Join(',', _additional)}]}}";
}

internal class Program
{
    public static void Main()
    {
        // Fuck this bullshit
        Thread.CurrentThread.CurrentCulture = System.Globalization.CultureInfo.CreateSpecificCulture("en-US");

        var da = new V2RDataArray("Ohio", (3, .5f), (2, .3f), Funcs.RotatedVecAtPoint);
        Console.WriteLine($"da={da.ToLongString()}");

        foreach (var point in da)
            Console.WriteLine($"Point: {point}");
        Console.WriteLine($"Key: {da.Key}, Count: {da.Count}, MinMax: {da.MinMax}, Nearest (inside (0.5f, 0.5f)): {da.Nearest(0.5f, 0.5f)}, Nearest (outside (-100.0f, 100.0f)): {da.Nearest(-100.0f, 100.0f)}");

        Console.WriteLine();

        var dl = new V2RList("Ohio", (3, .5f), (2, .3f), Funcs.RotatedVecAtPoint, 5, Funcs.DataItemFAtIndex);
        Console.WriteLine($"dl={dl.ToLongString()}");

        foreach (var point in dl)
            Console.WriteLine($"Point: {point}");
        Console.WriteLine($"Key: {dl.Key}, Count: {dl.Count}, MinMax: {dl.MinMax}, Nearest (inside (0.5f, 0.5f)): {dl.Nearest(0.5f, 0.5f)}, Nearest (outside (100.0f, -100.0f)): {dl.Nearest(100.0f, -100.0f)}");

        Console.WriteLine();

        var dd = new V2RDict("Ohio", (3, .5f), (2, .3f), Funcs.RotatedVecAtPoint, 10, Funcs.KeyDataItemFPairAtIndex);
        Console.WriteLine($"dd={dd.ToLongString()}");

        foreach (var point in dd)
            Console.WriteLine($"Point: {point}");
        Console.WriteLine($"Key: {dd.Key}, Count: {dd.Count}, MinMax: {dd.MinMax}, Nearest (inside (0.5f, 0.5f)): {dd.Nearest(0.5f, 0.5f)}, Nearest (outside (-50.0f, 50.0f)): {dd.Nearest(-50.0f, 50.0f)}");

        Console.WriteLine();

        var infos = new IDataInfo[3] { da, dl, dd };

        foreach (var info in infos)
            Console.WriteLine($"Key: {info.Key}, Count: {info.Count}, MinMax: {info.MinMax}, Nearest (inside (0.5f, 0.5f)): {info.Nearest(0.5f, 0.5f)}, Nearest (outside (-50.0f, 50.0f)): {info.Nearest(-50.0f, 50.0f)}");
    }
}
