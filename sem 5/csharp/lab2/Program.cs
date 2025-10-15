using System.Numerics;
using System.Text;

struct DataItemF(float x, float y, Vector2 v)
{
    public float X = x, Y = y;
    public Vector2 Data = v;

    public override readonly string ToString()
        => $"DataItemF {{ x={X}, y={Y}, data={Data} }}";
}

internal interface ILongStringifiable
{
    public string ToLongString();
}

static class Funcs
{
    public static Vector2 RotatedVecAtPoint(float x, float y) =>
        new(y, -x);

    public static DataItemF DataItemFAtIndex(int idx) =>
        new(idx, -idx, new Vector2(-idx, idx));

    public static ((float X, float Y), DataItemF Data) KeyDataItemFPairAtIndex(int idx) =>
        new((-idx * idx, idx * idx), new DataItemF(-idx * idx, idx * idx, new(-idx, idx)));

    public static float EuclidianDistance((float x, float y) p1, (float x, float y) p2)
    {
        return MathF.Sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
    }
};

interface IDataInfo
{
    string Key { get; set; }
    int Count { get; }
    (float Min, float Max) MinMax { get; }
    DataItemF Nearest(float x, float y);
}

class V2RDataArray : IDataInfo, IEnumerable<DataItemF>, ILongStringifiable
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
        int xIdx = Math.Clamp((int)MathF.Round(x / Xgrid.step), 0, Xgrid.size - 1);
        int yIdx = Math.Clamp((int)MathF.Round(y / Ygrid.step), 0, Ygrid.size - 1);
        float xCoord = xIdx * Xgrid.step;
        float yCoord = yIdx * Ygrid.step;
        return new DataItemF(xCoord, yCoord, PackedData[xIdx, yIdx]);
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

    public string GridToString()
    {
        var result = new StringBuilder("[");
        for (int i = 0; i < Xgrid.size; i++)
        {
            for (int j = 0; j < Ygrid.size; j++)
                result.Append($"(idx=({i}, {j}), coord=({i * Xgrid.step}, {j * Ygrid.step}), vec={PackedData[i, j]}), ");
        }
        if (Ygrid.size > 0 && Xgrid.size > 0)
            result.Length -= 2;

        result.Append(']');
        return result.ToString();
    }

    private string GetBasicInfo()
        => $"Key={Key}, Xgrid={Xgrid}, Ygrid={Ygrid}";

    private string GetAdditionalInfo()
        => $"data={GridToString()}";

    public override string ToString() =>
        $"V2DataArray {{ {GetBasicInfo()} }}";

    public virtual string ToLongString() =>
        $"V2DataArray {{ {GetBasicInfo()}, {GetAdditionalInfo()} }}";
}

class V2RList(string key, (int, float) Xgrid, (int, float) Ygrid,
               Func<float, float, Vector2> FR, int nList,
               Func<int, DataItemF> FL) : V2RDataArray(key, Xgrid, Ygrid, FR), IDataInfo, IEnumerable<DataItemF>, ILongStringifiable
{
    private readonly List<DataItemF> _additional = [
        .. Enumerable
        .Range(0, nList)
        .Select(i => FL(i))
    ];
    public new string Key { get; set; } = key;
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
        return Funcs.EuclidianDistance((minList.X, minList.Y), (x, y)) > Funcs.EuclidianDistance((minGrid.X, minGrid.Y), (x, y))
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

    public void Add(DataItemF item)
    {
        _additional.Add(item);
    }

    private string GetBasicInfo()
        => $"Key={Key}, Xgrid={Xgrid}, Ygrid={Ygrid}, Length={_additional.Count}";

    private string GetAdditionalInfo()
        => $"data={GridToString()}, additional=[{string.Join(", ", _additional)}]";

    public override string ToString() =>
        $"V2RList {{ {GetBasicInfo()} }}";

    public override string ToLongString() =>
        $"V2RList {{ {GetBasicInfo()}, {GetAdditionalInfo()} }}";
}

class V2RDict : IDataInfo, IEnumerable<DataItemF>, ILongStringifiable
{
    private readonly V2RDataArray _grid;
    private readonly Dictionary<(float X, float Y), DataItemF> _additional;

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
        var minDict = _additional.MinBy(p => Funcs.EuclidianDistance((p.Key.X, p.Key.Y), (x, y))).Value;
        var minGrid = _grid.Nearest(x, y);
        return Funcs.EuclidianDistance((minDict.X, minDict.Y), (x, y)) > Funcs.EuclidianDistance((minGrid.X, minGrid.Y), (x, y))
            ? minGrid
            : minDict;
    }

    System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
    {
        return GetEnumerator();
    }

    public IEnumerator<DataItemF> GetEnumerator()
    {
        foreach (var item in _grid)
            yield return item;
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

    private string GetBasicInfo()
        => $"Key={Key}, Xgrid={_grid.Xgrid}, Ygrid={_grid.Ygrid}, Length={_additional.Count}";

    private string GetAdditionalInfo()
        => $"data={_grid.GridToString()}, additional=[{string.Join(", ", _additional)}]";

    public override string ToString() =>
        $"V2RDict {{ {GetBasicInfo()} }}";

    public string ToLongString() =>
        $"V2RDict {{ {GetBasicInfo()}, {GetAdditionalInfo()} }}";
}

internal class Program
{
    private static void PrintDataInfo<T>(T v2Object, (float x, float y) nearestInside, (float x, float y) nearestOutside)
        where T : IDataInfo
    {
        Console.WriteLine($"Key: {v2Object.Key}\nCount: {v2Object.Count}\nMinMax: {v2Object.MinMax}"
                          + $"\nNearest (inside {nearestInside}): {v2Object.Nearest(nearestInside.x, nearestInside.y)}"
                          + $"\nNearest (outside {nearestOutside}): {v2Object.Nearest(nearestOutside.x, nearestOutside.y)}\n");        
    }

    private static void PrintObject<T>(string name, T v2Object, (float x, float y) nearestInside, (float x, float y) nearestOutside)
        where T : IDataInfo, IEnumerable<DataItemF>, ILongStringifiable
    {
        Console.WriteLine($"{name}={v2Object.ToLongString()}");
        foreach (var point in v2Object)
            Console.WriteLine($"- Point: {point}");
        PrintDataInfo(v2Object, nearestInside, nearestOutside);
    }

    public static void Main()
    {
        // Fuck this bullshit
        Thread.CurrentThread.CurrentCulture = System.Globalization.CultureInfo.CreateSpecificCulture("en-US");

        var da = new V2RDataArray("Ohio", (3, .5f), (2, .3f), Funcs.RotatedVecAtPoint);
        PrintObject("da", da, (0.5f, 0.5f), (-100.0f, 100.0f));

        var dl = new V2RList("Ohio", (3, .5f), (2, .3f), Funcs.RotatedVecAtPoint, 5, Funcs.DataItemFAtIndex);
        PrintObject("dl", dl, (0.5f, 0.5f), (100.0f, -100.0f));
        
        var dd = new V2RDict("Ohio", (3, .5f), (2, .3f), Funcs.RotatedVecAtPoint, 10, Funcs.KeyDataItemFPairAtIndex);
        PrintObject("dd", dd, (0.5f, 0.5f), (-50.0f, 50.0f));
        
        var infos = new IDataInfo[3] { da, dl, dd };
        foreach (var info in infos)
            PrintDataInfo(info, (0.5f, 0.5f), (-50.0f, 50.0f));
    }
}
