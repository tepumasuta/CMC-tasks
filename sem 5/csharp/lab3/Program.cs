using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Numerics;
using System.Reflection;
using System.Collections.Specialized;

class DataItemFT : INotifyPropertyChanged
{
    public event PropertyChangedEventHandler? PropertyChanged;
    public float X { get; set; }
    public float Y { get; set; }
    public Vector2 Data
    {
        get => _data;
        set
        {
            _data = value;
            OnPropertyChanged(nameof(Data));
        }
    }

    public DateTime Measurement
    {
        get => _measurement;
        set
        {
            _measurement = value;
            OnPropertyChanged(nameof(Measurement));
        }
    }

    private Vector2 _data;
    private DateTime _measurement;

    public DataItemFT()
    {
        X = 0;
        Y = 0;
        _data = new Vector2();
        _measurement = DateTime.Now;
    }
    
    public DataItemFT(float x, float y, Vector2 v, DateTime dm)
    {
        X = x;
        Y = y;
        _data = v;
        _measurement = dm;
    }

    public string ToSavedString() =>
        $"DataItemFT;{X};{Y};{_data.X};{_data.Y};{_measurement}";

    public static bool FromSavedString(string source, ref DataItemFT dt)
    {
        string[] parts = source.Split(";");
        if (parts is [string class_name, string x, string y, string vx, string vy, string measurement]
            && class_name == "DataItemFT")
        {
            try
            {
                dt = new DataItemFT(
                    float.Parse(x),
                    float.Parse(y),
                    new Vector2(float.Parse(vx), float.Parse(vy)),
                    DateTime.Parse(measurement)
                );
            }
            catch (Exception e)
            {
                Console.WriteLine($"[ERROR]: Failed to parse DataItemFT from string: {e}");
                return false;
            }
            return true;
        }
        return false;
    }

    public override string ToString()
    {
        return $"DataItemFT {{X = {X}, Y = {Y}, Data = {_data}, Measurement = {_measurement}}}";
    }

    private void OnPropertyChanged(string field_name)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(field_name));
    }
}

class DataItemEventArgs(string? property_name, string? value) : EventArgs
{
    public string? PropertyName { get; } = property_name;
    public string? Value { get; } = value;
}

class FCollection : ObservableCollection<DataItemFT>
{
    public event DataItemHandler? ItemChanged;
    public delegate void DataItemHandler(object? sender, DataItemEventArgs args);

    public new void Add(DataItemFT item)
    {
        base.Add(item);
        item.PropertyChanged += DataItemChanged;
    }
    public new bool Remove(DataItemFT item)
    {
        bool success = base.Remove(item);
        if (success) item.PropertyChanged -= DataItemChanged;
        return success;
    }
    public int AddRange(params DataItemFT[] dts)
    {
        foreach(var dt in dts)
            Add(dt);
        return dts.Length;
    }
    public new DataItemFT this[int j]
    {
        get => base[j];
        set
        {
            base[j].PropertyChanged -= DataItemChanged;
            base[j] = value;
            base[j].PropertyChanged += DataItemChanged;
        }
    }

    public bool Save(string filename)
    {
        try
        {
            using FileStream fs = new FileStream(filename, FileMode.OpenOrCreate);
            using BinaryWriter writer = new(fs);
            writer.Write(Count);
            foreach (var dt in this)
                writer.Write(dt.ToSavedString());
            return true;
        }
        catch (Exception e)
        {
            Console.WriteLine($"[ERROR]: Failed to save FCollection to file {filename}: {e.Message}");
            return false;
        }
    }
    public bool Load(string filename)
    {
        try
        {
            using FileStream fs = new(filename, FileMode.Open);
            using BinaryReader reader = new(fs);
            int count = reader.ReadInt32();
            DataItemFT[] tmp = new DataItemFT[count];
            for (int i = 0; i < count; i++)
            {
                DataItemFT tmp_item = new();
                if (DataItemFT.FromSavedString(reader.ReadString(), ref tmp_item))
                    tmp[i] = tmp_item;
                else
                    return false;
            }
            Clear();
            AddRange(tmp);
            return true;
        }
        catch (Exception e)
        {
            Console.WriteLine($"[ERROR]: Failed to load FCollection to file {filename}: {e.Message}");
            return false;
        }
    }

    public IEnumerable<DataItemFT> FindByTimeYear(int year) =>
        from item in this where item.Measurement.Year == year select item;
    public IEnumerable<float>? OrderedCoords
    {
        get {
            var query = from item in this
                        group item by item.X into xs
                        let count = xs.Count()
                        where count == 1
                        orderby xs.Key
                        select xs.Key;
            return query.Any() ? query : null;
        }
    }

    public override string ToString() =>
        $"FCollection {{items = [{string.Join(',', this)}]}}";

    private new void Clear()
    {
        foreach (var dt in this)
            dt.PropertyChanged -= DataItemChanged;
        base.Clear();
    }

    private void DataItemChanged(object? sender, PropertyChangedEventArgs args)
    {
        Type type = typeof(DataItemFT);
        PropertyInfo? field_info = null;
        if (args.PropertyName != null)
            field_info = type.GetProperty(args.PropertyName);
        OnItemChanged(args.PropertyName, field_info?.GetValue(sender)?.ToString());
    }

    private void OnItemChanged(string? property_name, string? value) =>
        ItemChanged?.Invoke(this, new DataItemEventArgs(property_name, value));
}

class Journal()
{
    public List<string> Log { get; } = [];
    public void LogAction(object? sender, NotifyCollectionChangedEventArgs args)
    {
        Log.Add(args.Action.ToString());
    }
    public void LogItemChange(object? sender, DataItemEventArgs args)
    {
        if (args.PropertyName != null)
            Log.Add($"{args.PropertyName} = {args.Value}");
    }
    public override string ToString() =>
        $"Journal = [{string.Join(",", Log)}]";
}

internal class Program
{
    public static void Main()
    {
        DataItemFT item = new(69, 42, new(13, 7), DateTime.Now.AddYears(5));
        string item_str = item.ToSavedString();
        Console.WriteLine(item_str);
        DataItemFT deserealized_item = new();
        Console.WriteLine(DataItemFT.FromSavedString(item_str, ref deserealized_item)
                          ? "Successfully deserealized DataItemFT"
                          : "Failed to deserealize DataItemFT");
        Console.WriteLine(deserealized_item.ToString());
        Console.WriteLine();


        FCollection collection = [];
        Journal journal = new();
        collection.CollectionChanged += journal.LogAction;
        collection.ItemChanged += journal.LogItemChange;
        
        collection.AddRange(
            new DataItemFT(1, 2, new Vector2(10, 20), new DateTime(2023, 1, 1)),
            new DataItemFT(5, 6, new Vector2(50, 60), new DateTime(2023, 3, 3)),
            new DataItemFT(3, 4, new Vector2(30, 40), new DateTime(2024, 2, 2)),
            new DataItemFT(5, 6, new Vector2(50, 60), new DateTime(2025, 3, 3))
        );

        Console.WriteLine(collection);
        Console.WriteLine(journal);

        collection.Remove(collection[2]);
        collection[0] = new DataItemFT(7, 8, new Vector2(70, 80), new DateTime(2024, 4, 4));
        collection[2].Data = new Vector2(99, 88);
        collection[2].Measurement = new DateTime(2025, 5, 5);

        Console.WriteLine(collection);
        Console.WriteLine(journal);
        Console.WriteLine();


        string filename = "test_collection.dat";
        Console.WriteLine(collection.Save(filename)
                          ? $"Successfully saved collection to file {filename}"
                          : $"Failed to save collection to file {filename}");
        FCollection restoredCollection = [];
        Console.WriteLine(restoredCollection.Load(filename)
                          ? $"Successfully loaded collection from file {filename}"
                          : $"Failed to load collection from file {filename}");
        Console.WriteLine(restoredCollection);
        Console.WriteLine();

        restoredCollection.AddRange(
            new DataItemFT(-1, 4, new Vector2(30, 40), new DateTime(2024, 2, 2)),
            new DataItemFT(0, 4, new Vector2(30, 40), new DateTime(2023, 2, 2))
        );

        Console.WriteLine(restoredCollection);

        Console.WriteLine("Query: FindByTimeYear(2023) - find all items with measurement year 2023");
        var yearQuery = restoredCollection.FindByTimeYear(2023);
        Console.WriteLine(yearQuery.Any() 
                          ? $"Result: {string.Join("; ", yearQuery)}" 
                          : "Result: No items found");

        Console.WriteLine("Query: OrderedCoords - get unique X coordinates in ascending order");
        var coordsQuery = restoredCollection.OrderedCoords;
        Console.WriteLine(coordsQuery != null 
                          ? $"Result: {string.Join(", ", coordsQuery)}" 
                          : "Result: No unique coordinates found");

        Console.WriteLine();


        DataItemFT error_item = new();
        string invalid_string = "Invalid;format;string";
        Console.WriteLine(DataItemFT.FromSavedString(invalid_string, ref error_item)
                          ? "Unexpectedly succeeded"
                          : "Expected failure - invalid format");
        Console.WriteLine();

        
        FCollection emptyCollection = [];
        Console.WriteLine(emptyCollection.Load("non_existent_file.dat")
                          ? "Unexpectedly loaded"
                          : "Expected failure - file not found");
        Console.WriteLine();


        FCollection testCollection1 = [];
        testCollection1.AddRange(
            new DataItemFT(1, 2, new Vector2(10, 20), new DateTime(2020, 1, 1)),
            new DataItemFT(3, 4, new Vector2(30, 40), new DateTime(2021, 2, 2))
        );
        
        Console.WriteLine("Query: FindByTimeYear(2023) on collection with years 2020, 2021");
        var emptyYearQuery = testCollection1.FindByTimeYear(2023);
        Console.WriteLine(emptyYearQuery.Any() 
                          ? $"Result: {string.Join("; ", emptyYearQuery)}" 
                          : "Result: No items found for year 2023");
        Console.WriteLine();


        FCollection testCollection2 = [];
        testCollection2.AddRange(
            new DataItemFT(1, 1, new Vector2(1, 1), DateTime.Now),
            new DataItemFT(1, 2, new Vector2(2, 2), DateTime.Now),
            new DataItemFT(2, 3, new Vector2(3, 3), DateTime.Now),
            new DataItemFT(2, 4, new Vector2(4, 4), DateTime.Now)
        );
        
        Console.WriteLine("Query: OrderedCoords on collection with all duplicate X values");
        var emptyCoordsQuery = testCollection2.OrderedCoords;
        Console.WriteLine(emptyCoordsQuery != null 
                          ? $"Result: {string.Join(", ", emptyCoordsQuery)}" 
                          : "Result: No unique coordinates found (all X values are duplicates)");
    }
}
