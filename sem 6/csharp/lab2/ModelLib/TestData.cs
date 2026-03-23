using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Text;
using System.IO;
using System.Linq;
using System.Collections.ObjectModel;

namespace ModelLib;

public class TestData
{
    private readonly string[] names = ["Smelyansky", "Chernov", "Abramov", "Kapustin", "Berezina", "Morozov", "Inovenkov", "Smirnov", "Shapovalov", "Makarov", "Vorontsov", "Polyakova", "Bobyleva", "Voronenko", "Nefedov"];

    public List<string> FiringList { set; get; }
    public DateTime When { set; get; }
    public bool IsSaved { get; set; }
    public string Substring { get; set; }

    public TestData()
    {
        FiringList = [];
        When = DateTime.Today;
        IsSaved = false;
        Substring = string.Empty;
    }

    public TestData(int count)
    {
        When = DateTime.Today;
        IsSaved = false;
        Substring = string.Empty;
        if (count > names.Length)
            throw new ArgumentOutOfRangeException(nameof(count), $"Count cannot exceed {names.Length}. Provided: {count}.");
        FiringList = [.. names[..count]];
    }

    public IEnumerable<string> Subset(string substring)
    {
        if (string.IsNullOrEmpty(substring))
            return [];
        return FiringList.Where(s => s.Contains(substring, StringComparison.OrdinalIgnoreCase));
    }

    public void SaveData(string filename)
    {
        try
        {
            using FileStream fs = new(filename, FileMode.OpenOrCreate);
            using StreamWriter writer = new(fs);
            writer.WriteLine(When.ToBinary());
            writer.WriteLine(Substring ?? "");
            foreach (var dt in FiringList)
                writer.WriteLine(dt);
            IsSaved = true;
        }
        catch (Exception e)
        {
            Console.WriteLine($"[WARNING]: Exception {e}");
            throw;
        }
    }

    public static void LoadData(string filename, ref TestData tdata)
    {
        try
        {
            using FileStream fs = new(filename, FileMode.Open);
            using StreamReader reader = new(fs);
            DateTime date = DateTime.FromBinary(Convert.ToInt64(reader.ReadLine()));
            string substring = reader.ReadLine() ?? "";
            List<string> tmpList = [];
            string? line;
            while ((line = reader.ReadLine()) != null)
            {
                tmpList.Add(line);
            }
            var tmp = new TestData
            {
                When = date,
                FiringList = tmpList,
                Substring = substring
            };
            tdata = tmp;
        }
        catch (Exception e)
        {
            Console.WriteLine($"[WARNING]: Exception {e}");
            throw;
        }
    }

    public override string ToString()
        => $"TestData {{ IsSaved = {IsSaved}, When = {When}, FiringList = [{string.Join(", ", FiringList)}] }}";
}

public class DataItem : INotifyPropertyChanged
{
    private string _name;
    private DateTime _date;
    private int _n;
    private (double from, double to) _bounds;
    private List<double> _values;

    public event PropertyChangedEventHandler? PropertyChanged;

    public string Name
    {
        get => _name;
        set => SetField(ref _name, value);
    }

    public DateTime Date
    {
        get => _date;
        set => SetField(ref _date, value);
    }

    public int N
    {
        get => _n;
        set => SetField(ref _n, value);
    }

    public (double from, double to) Bounds
    {
        get => _bounds;
        set => SetField(ref _bounds, value);
    }

    public List<double> Values
    {
        get => _values;
        set => SetField(ref _values, value);
    }

    public DataItem(string name, DateTime date, int n, (double, double) bounds, Func<double, double> f)
    {
        _name = name;
        _date = date;
        _n = n;
        _bounds = bounds;
        _values = [];

        InitializeValues(f);
    }

    public DataItem()
    {
        _name = "Default item";
        _date = DateTime.Today;
        _n = 10;
        _bounds = (0.0, 1.0);
        _values = [];

        InitializeValues(x => x * x);
    }

    private void InitializeValues(Func<double, double> f)
    {
        if (N <= 0)
            return;

        double step = (Bounds.to - Bounds.from) / N;
        List<double> tmpList = [.. Enumerable.Range(0, N).Select((x, i) => f(Bounds.from + i * step))];
        Values = tmpList;
    }

    public override string ToString() =>
        $"DataItem {{Name={Name}, Date={Date}, N={N}, Bounds={{from={Bounds.from}, to={Bounds.to}}}, Values={string.Join(", ", Values)}}}";

    protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    protected bool SetField<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value))
            return false;

        field = value;
        OnPropertyChanged(propertyName);
        return true;
    }
}

public class DataCollection : ObservableCollection<DataItem>
{
    public DataCollection()
    {
        Add(new DataItem("sin(x)", DateTime.Today, 10, (0.0, Math.PI), Math.Sin));
        Add(new DataItem("cos(x)", DateTime.Today.AddDays(-1), 8, (0.0, Math.PI), Math.Cos));
        Add(new DataItem("x^2", DateTime.Today.AddDays(-2), 6, (-2.0, 2.0), x => x * x));
    }

    public void UpdateCollection()
    {
        if (Count > 0)
        {
            var updated = new DataItem("updated item", DateTime.Now, 12, (-3.0, 3.0), x => x * x + 2 * x + 1);

            var item = this[^1];
            item.Name = updated.Name;
            item.Date = updated.Date;
            item.N = updated.N;
            item.Bounds = updated.Bounds;
            item.Values = updated.Values;
        }

        Add(new DataItem("new item", DateTime.Now, 5, (0.0, 5.0), x => x + 1));
    }

    public override string ToString() =>
        $"DataCollection = [{string.Join(", ", this)}]";
}

