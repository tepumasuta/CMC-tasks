using Avalonia.Controls;
using Avalonia.Interactivity;
using ModelLib;

namespace Lab2App;

public partial class MainWindow : Window
{
    public DataCollection Items { get; } = [];

    public MainWindow()
    {
        InitializeComponent();
        DataContext = this;
    }

    private void UpdateButton_OnClick(object? sender, RoutedEventArgs e)
    {
        Items.UpdateCollection();
    }

}