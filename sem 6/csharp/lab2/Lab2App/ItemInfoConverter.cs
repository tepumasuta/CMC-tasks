using System;
using System.Collections.Generic;
using System.Globalization;
using Avalonia.Data;
using Avalonia.Data.Converters;

namespace Lab2App;

public sealed class ItemInfoConverter : IMultiValueConverter
{
    public object? Convert(IList<object?> values, Type targetType, object? parameter, CultureInfo culture)
    {
        if (values.Count != 2) return BindingOperations.DoNothing;
        if (values[0] is not int n) return BindingOperations.DoNothing;
        if (values[1] is not ValueTuple<double, double> bounds) return BindingOperations.DoNothing;

        return $"N = {n}, Bounds = ({bounds.Item1}; {bounds.Item2})";
    }
}