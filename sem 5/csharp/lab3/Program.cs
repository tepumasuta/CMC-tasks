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

    public override string ToString() =>
        $"DataItemFT(X={X}, Y={Y}, Data=({_data.X}, {_data.Y}), Measurement={_measurement:yyyy-MM-dd})";

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
            using FileStream fs = new(filename, FileMode.OpenOrCreate);
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
        Count == 0 
        ? "FCollection [<empty>]" 
        : $"FCollection [\n{string.Join(",\n", this.Select((item, index) => $"    [{index}] {item}"))}\n]";

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
        Log.Count == 0 
        ? "Journal [<empty>]" 
        : $"Journal [\n{string.Join(",\n", Log.Select((entry, index) => $"    {index + 1}. {entry}"))}\n]";}

internal class Program
{
    public static void Main()
    {
        Console.WriteLine("DATA ITEM SERIALIZATION TEST");
        Console.WriteLine("=============================");
        DataItemFT item = new(69, 42, new(13, 7), DateTime.Now.AddYears(5));
        string item_str = item.ToSavedString();
        Console.WriteLine("Serialized: " + item_str);
        
        DataItemFT deserialized_item = new();
        bool success = DataItemFT.FromSavedString(item_str, ref deserialized_item);
        
        Console.WriteLine(success ? "SUCCESS: DataItemFT deserialized" : "FAILED: DataItemFT deserialization");
        Console.WriteLine("Deserialized: " + deserialized_item);
        Console.WriteLine();

        // Test collection functionality
        Console.WriteLine("COLLECTION AND JOURNAL TEST");
        Console.WriteLine("===========================");
        FCollection collection = [];
        Journal journal = new();
        
        collection.CollectionChanged += journal.LogAction;
        collection.ItemChanged += journal.LogItemChange;
        
        Console.WriteLine("Initial state:");
        Console.WriteLine(collection);
        Console.WriteLine(journal);
        Console.WriteLine();

        Console.WriteLine("Adding initial items...");
        collection.AddRange(
            new DataItemFT(1, 2, new Vector2(10, 20), new DateTime(2023, 1, 1)),
            new DataItemFT(5, 6, new Vector2(50, 60), new DateTime(2023, 3, 3)),
            new DataItemFT(3, 4, new Vector2(30, 40), new DateTime(2024, 2, 2)),
            new DataItemFT(5, 6, new Vector2(50, 60), new DateTime(2025, 3, 3))
        );

        Console.WriteLine("After adding items:");
        Console.WriteLine(collection);
        Console.WriteLine(journal);
        Console.WriteLine();

        Console.WriteLine("MODIFYING COLLECTION...");
        collection.Remove(collection[2]);
        collection[0] = new DataItemFT(7, 8, new Vector2(70, 80), new DateTime(2024, 4, 4));
        collection[2].Data = new Vector2(99, 88);
        collection[2].Measurement = new DateTime(2025, 5, 5);

        Console.WriteLine("After modifications:");
        Console.WriteLine(collection);
        Console.WriteLine(journal);
        Console.WriteLine();

        // Test save/load functionality
        Console.WriteLine("FILE SAVE/LOAD TEST");
        Console.WriteLine("===================");
        string filename = "test_collection.dat";
        bool saveSuccess = collection.Save(filename);
        
        Console.WriteLine(saveSuccess ? "SUCCESS: Collection saved to " + filename : "FAILED: Save to " + filename);
                          
        FCollection restoredCollection = [];
        bool loadSuccess = restoredCollection.Load(filename);
        
        Console.WriteLine(loadSuccess ? "SUCCESS: Collection loaded from " + filename : "FAILED: Load from " + filename);
        Console.WriteLine("Restored collection:");
        Console.WriteLine(restoredCollection);
        Console.WriteLine();

        // Add more items and test queries
        Console.WriteLine("QUERY TESTS");
        Console.WriteLine("===========");
        Console.WriteLine("Adding more items to restored collection...");
        restoredCollection.AddRange(
            new DataItemFT(-1, 4, new Vector2(30, 40), new DateTime(2024, 2, 2)),
            new DataItemFT(0, 4, new Vector2(30, 40), new DateTime(2023, 2, 2))
        );

        Console.WriteLine("Restored collection with new items:");
        Console.WriteLine(restoredCollection);
        Console.WriteLine();

        Console.WriteLine("QUERY 1: FindByTimeYear(2023)");
        var yearQuery = restoredCollection.FindByTimeYear(2023);
        
        if (yearQuery.Any())
        {
            Console.WriteLine($"Found {yearQuery.Count()} items for year 2023:");
            var queryResults = string.Join(",\n", yearQuery.Select(item => $"    {item}"));
            Console.WriteLine($"[\n{queryResults}\n]");
        }
        else
        {
            Console.WriteLine("No items found for year 2023");
        }
        Console.WriteLine();

        Console.WriteLine("QUERY 2: OrderedCoords");
        var coordsQuery = restoredCollection.OrderedCoords;
        
        if (coordsQuery != null)
        {
            Console.WriteLine("Unique X coordinates in ascending order:");
            var coordsList = string.Join(", ", coordsQuery);
            Console.WriteLine($"[ {coordsList} ]");
        }
        else
        {
            Console.WriteLine("No unique coordinates found");
        }
        Console.WriteLine();

        // Test error handling
        Console.WriteLine("ERROR HANDLING TESTS");
        Console.WriteLine("====================");
        
        Console.WriteLine("Testing invalid string parsing...");
        DataItemFT error_item = new();
        string invalid_string = "Invalid;format;string";
        bool parseSuccess = DataItemFT.FromSavedString(invalid_string, ref error_item);
        Console.WriteLine(parseSuccess ? "UNEXPECTED: Should have failed" : "EXPECTED: Failed to parse invalid format");
        Console.WriteLine();

        Console.WriteLine("Testing file not found...");
        FCollection emptyCollection = [];
        bool loadFail = emptyCollection.Load("non_existent_file.dat");
        Console.WriteLine(loadFail ? "UNEXPECTED: Should have failed" : "EXPECTED: File not found");
        Console.WriteLine();

        // Test empty query results
        Console.WriteLine("EDGE CASE TESTS");
        Console.WriteLine("===============");
        
        FCollection testCollection1 = [];
        testCollection1.AddRange(
            new DataItemFT(1, 2, new Vector2(10, 20), new DateTime(2020, 1, 1)),
            new DataItemFT(3, 4, new Vector2(30, 40), new DateTime(2021, 2, 2))
        );
        
        Console.WriteLine("Testing FindByTimeYear(2023) on collection with years 2020-2021:");
        var emptyYearQuery = testCollection1.FindByTimeYear(2023);
        
        if (emptyYearQuery.Any())
        {
            var results = string.Join(",\n", emptyYearQuery.Select(item => $"    {item}"));
            Console.WriteLine($"[\n{results}\n]");
        }
        else
        {
            Console.WriteLine("[<no results>]");
        }
        Console.WriteLine();

        // Test duplicate coordinates scenario
        FCollection testCollection2 = [];
        testCollection2.AddRange(
            new DataItemFT(1, 1, new Vector2(1, 1), DateTime.Now),
            new DataItemFT(1, 2, new Vector2(2, 2), DateTime.Now),
            new DataItemFT(2, 3, new Vector2(3, 3), DateTime.Now),
            new DataItemFT(2, 4, new Vector2(4, 4), DateTime.Now)
        );
        
        Console.WriteLine("Testing OrderedCoords with duplicate X values:");
        var emptyCoordsQuery = testCollection2.OrderedCoords;
        
        if (emptyCoordsQuery != null)
        {
            Console.WriteLine($"[ {string.Join(", ", emptyCoordsQuery)} ]");
        }
        else
        {
            Console.WriteLine("[<no unique coordinates>]");
        }

        Console.WriteLine();
        Console.WriteLine("ALL TESTS COMPLETED");
        Console.WriteLine("===================");    }
}
