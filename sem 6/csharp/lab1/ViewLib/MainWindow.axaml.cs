using ModelLib;

using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Platform.Storage;
using MsBox.Avalonia;
using MsBox.Avalonia.Enums;

using System;
using System.Threading.Tasks;
using System.IO;

namespace ViewLib;

public partial class MainWindow : Window
{
    private TestData _testData;
    private string? _currentFileName;

    public MainWindow()
    {
        InitializeComponent();
        _testData = new(5);
    }

    private void OnLoaded(object sender, RoutedEventArgs e)
    {
        UpdateUIFromTestData();
    }

    private async void OnClosing(object sender, WindowClosingEventArgs e)
    {

        if (!e.IsProgrammatic && !_testData.IsSaved)
        {
            e.Cancel = true;
            var result = await MessageBoxManager.GetMessageBoxStandard(
                "Подтверждение",
                "Данные не сохранены, сохранить перед выходом?",
                ButtonEnum.YesNo
            ).ShowAsync();

            if (result == ButtonResult.Yes)
            {
                await SaveDataAsync();

                if (!_testData.IsSaved)
                    Console.WriteLine("[ERROR]: Failed to save data on closing");
            }

            if (result != ButtonResult.None)
            {
                Close();
            }
        }
    }

    private void OutputDataButton_Click(object sender, RoutedEventArgs e)
    {
        UpdateUIFromTestData();
    }

    private async void LoadButton_Click(object sender, RoutedEventArgs e)
    {
        await LoadDataAsync();
    }

    private async void SaveButton_Click(object sender, RoutedEventArgs e)
    {
        await SaveDataAsync();
    }

    private async Task SaveDataAsync()
    {
        try
        {
            var file = await StorageProvider.SaveFilePickerAsync(
                new FilePickerSaveOptions
                {
                    Title = "Сохранить файл",
                    DefaultExtension = "txt",
                    SuggestedFileName = "fired.txt",
                    SuggestedStartLocation = await StorageProvider.TryGetFolderFromPathAsync(
                        new Uri(Directory.GetCurrentDirectory())
                    )
                }
            );

            if (file == null) return;
            _currentFileName = file.Path.LocalPath;

            _testData.SaveData(_currentFileName);
            UpdateUIFromTestData();
        }
        catch (Exception ex)
        {
            await MessageBoxManager.GetMessageBoxStandard(
                "Ошибка сохранения",
                ex.Message,
                ButtonEnum.Ok
            ).ShowAsync();
        }
    }

    private async Task LoadDataAsync()
    {
        try
        {
            var files = await StorageProvider.OpenFilePickerAsync(
                new FilePickerOpenOptions
                {
                    Title = "Выберите файл для загрузки",
                    AllowMultiple = false
                }
            );

            if (files.Count >= 1)
            {
                var file = files[0].Path.LocalPath;

                TestData.LoadData(file, ref _testData);

                _currentFileName = file;
                UpdateUIFromTestData();
                FileNameTextBlock.Text = _currentFileName;
            }
        }
        catch (Exception ex)
        {
            await MessageBoxManager.GetMessageBoxStandard(
                "Ошибка загрузки",
                ex.Message,
                ButtonEnum.Ok
            ).ShowAsync();
        }
    }
    private void UpdateUIFromTestData()
    {
        FiringListBox.ItemsSource = _testData.FiringList;
        WhenDatePicker.SelectedDate = _testData.When;
        ToStringTextBlock.Text = _testData.ToString();
        FileNameTextBlock.Text = _currentFileName ?? "[файл не загружен]";
        SaveButton.IsEnabled = !_testData.IsSaved;
    }
}
