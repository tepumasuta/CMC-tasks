using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

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
