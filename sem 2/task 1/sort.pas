program SortComparison;

const
  MAX_INTEGER_VALUE = 100;
  MIN_INTEGER_VALUE = 0;
  MAX_ARRAY_SIZE = 10000;
  COMPRESSED_DEBUG_PRINT = true;
  STATIC_TEST = true; { Controls wheter perform testcases on user input or predefined data }

type
  IntArray = array[1..MAX_ARRAY_SIZE] of Integer;
  ArrayType = (SortedT,ReversedT,RandomizedT);
  SortType = (InsertT,HeapT);
  DebugInfo = record
    initialState: ArrayType;
    sortType: SortType;
    size: Integer;
    swaps: LongInt;
    comparisons: LongInt;
  end;

var
  info: DebugInfo;
  generatedArray, testingArray: IntArray;
  n: Integer;

function IsNonDescending(var arr: IntArray; size: Integer): Boolean;
var
  i: Integer;
begin
  isNonDescending := true;
  for i := 1 to size - 1 do
    if arr[i + 1] < arr[i] then
      isNonDescending := false;
end;

function RandomInt: Integer;
begin
  RandomInt := Random(MAX_INTEGER_VALUE - MIN_INTEGER_VALUE) + MIN_INTEGER_VALUE;
end;

procedure GenerateArray(var arr: IntArray; size: Integer; initialState: ArrayType);
var
  i: Integer;
begin{
  case initialState of
    SortedT: for i := 1 to size do arr[i] := i;
    ReversedT: for i := 1 to size do arr[i] := size - i + 1;
    RandomizedT: for i := 1 to size do arr[i] := RandomInt;
  end;}
  for i := 1 to size do arr[i] := 1;
end;

procedure PrintArray(var arr: IntArray; size: Integer);
var
  i: Integer;
begin
  Write('[');
  for i := 1 to size-1 do
    Write(arr[i], ' ');
  WriteLn(arr[size], ']');
end;

procedure InitializeState;
begin
  Randomize();
end;

procedure ResetDebugInfo(initialState: ArrayType; size: Integer);
begin
  info.size := size;
  info.initialState := initialState;
  info.comparisons := 0;
  info.swaps := 0;
end;

{ Copies `from` array into `into` array }
procedure ResetTestcase(var from, into: IntArray; size: Integer);
var
  i: Integer;
begin
  for i := 1 to size do
    into[i] := from[i];
end;

{ Performs actions on each swap. In particular increments current swaps count
  * May be used to print array on each swap
}
procedure OnSwap;
begin
  Inc(info.swaps);
end;

{ Performs actions on each comparison. In particular increments current comparisons count
  * May be used to print array on each comparison
}
procedure OnComparison;
begin
  Inc(info.comparisons);
end;

procedure Swap(var a, b: Integer);
var
  tmp: Integer;
begin
  tmp := a;
  a := b;
  b := tmp;
end;

{ Sorts given array `arr` with length `size` in-place via insert sort }
procedure InsertSort(var arr: IntArray; size: Integer);
var
  i, j: Integer;
begin
  info.sortType := InsertT;
  for i := 1 to size do
    for j := i downto 2 do begin
      OnComparison;
      if arr[j - 1] > arr[j] then begin
        OnSwap;
        Swap(arr[j - 1], arr[j]);
      end else
        break;
    end;
end;

{ Performs downfall of an element at the top given almost sorted heap (only first element out of order) }
procedure Heapify(var arr: IntArray; size: Integer);
var
  inOrder: Boolean;
  i, left, right: Integer;
begin
  inOrder := false;
  i := 1;
  while not inOrder do begin
    left := 2 * i;
    right := 2 * i + 1;
    if left > size then
      inOrder := true
    else if right > size then begin
      OnComparison;
      if arr[i] >= arr[left] then
        inOrder := true
      else begin
        OnSwap;
        Swap(arr[i], arr[left]);
        i := left;
      end;
    end else begin
      OnComparison;
      OnComparison;
      if arr[left] > arr[right] then begin
        if arr[i] >= arr[left] then
          inOrder := true
        else begin
          OnSwap;
          Swap(arr[i], arr[left]);
          i := left;
        end;
      end else begin
        if arr[i] >= arr[right] then
          inOrder := true
        else begin
          OnSwap;
          Swap(arr[i], arr[right]);
          i := right;
        end;
      end;
    end;
  end;
end;

{ Build max heap in `arr` with length `size` in-place }
procedure BuildHeap(var arr: IntArray; size: Integer);
var
  i, j: Integer;
begin
  for i := 1 to size do begin
    j := i;
    OnComparison;
    while (j <> 1) and (arr[j div 2] < arr[j]) do begin
      OnSwap;
      Swap(arr[j div 2], arr[j]);
      j := j div 2;
      OnComparison;
    end;
  end;
end;

{ Sorts given array `arr` with given length `size` via heap sort in-place }
procedure HeapSort(var arr: IntArray; size: Integer);
var
  i: Integer;
begin
  info.sortType := HeapT;
  BuildHeap(arr, size);
  for i := size downto 2 do begin
    OnSwap;
    Swap(arr[i], arr[1]);
    Heapify(arr, i - 1);
  end;
end;

procedure PrintDebugInfo(compressed: Boolean);
begin
  if compressed then begin
    Write('(');
    case info.initialState of
      SortedT: Write('Sort');
      ReversedT: Write('Revr');
      RandomizedT: Write('Rand');
    end;
    Write(',');
    case info.sortType of
      InsertT: Write('Insr');
      HeapT: Write('Heap');
    end;
    WriteLn(',', info.size, ',', info.comparisons, ',', info.swaps, ')');
  end else begin
    Write('Initially: ');
    case info.initialState of
      SortedT: WriteLn('Sorted');
      ReversedT: WriteLn('Reversed');
      RandomizedT: WriteLn('Randomized');
    end;
    Write('Type: ');
    case info.sortType of
      InsertT: WriteLn('Insert sort');
      HeapT: WriteLn('Heap sort');
    end;
    WriteLn('Size: ', info.size);
    WriteLn('Comparisons: ', info.comparisons);
    WriteLn('Swaps: ', info.swaps);
    WriteLn;
  end;
end;

{ Adds up to total swaps and comparisons per sort }
procedure CollectData(var totalComparisons, totalSwaps: LongInt);
begin
  totalComparisons := totalComparisons + info.comparisons;
  totalSwaps := totalSwaps + info.swaps;
end;

{ Requires GenerateArray }
procedure TestInsertSort(size: Integer; initialState: ArrayType; var totalComparisons, totalSwaps: LongInt);
begin
  ResetDebugInfo(initialState, size);
  ResetTestcase(generatedArray, testingArray, size);
  InsertSort(testingArray, size);
  PrintDebugInfo(COMPRESSED_DEBUG_PRINT);
  CollectData(totalComparisons, totalSwaps);
end;

{ Requires GenerateArray }
procedure TestHeapSort(size: Integer; initialState: ArrayType; var totalComparisons, totalSwaps: LongInt);
begin
  ResetDebugInfo(initialState, size);
  ResetTestcase(generatedArray, testingArray, size);
  HeapSort(testingArray, size);
  PrintDebugInfo(COMPRESSED_DEBUG_PRINT);
  CollectData(totalComparisons, totalSwaps);
end;

procedure TestWithSize(size: Integer);
var
  totalComparisonsInsert: LongInt;
  totalComparisonsHeap: LongInt;
  totalSwapsInsert: LongInt;
  totalSwapsHeap: LongInt;
begin
  totalComparisonsInsert := 0;
  totalComparisonsHeap := 0;
  totalSwapsInsert := 0;
  totalSwapsHeap := 0;

  { Common testcase format:
    - Generate array
    - Test sorts on this array
  }

  GenerateArray(generatedArray, size, SortedT);
  PrintArray(generatedArray, size);
  TestHeapSort(size, SortedT, totalComparisonsHeap, totalSwapsHeap);
  PrintArray(testingArray, size);
  TestInsertSort(size, SortedT, totalComparisonsInsert, totalSwapsInsert);
  PrintArray(testingArray, size);

  size := 8;

  GenerateArray(generatedArray, size, ReversedT);
  PrintArray(generatedArray, size);
  TestHeapSort(size, ReversedT, totalComparisonsHeap, totalSwapsHeap);
  PrintArray(testingArray, size);
  TestInsertSort(size, ReversedT, totalComparisonsInsert, totalSwapsInsert);
  PrintArray(testingArray, size);

  size := 10;

  GenerateArray(generatedArray, size, RandomizedT);
  PrintArray(generatedArray, size);
  TestHeapSort(size, RandomizedT, totalComparisonsHeap, totalSwapsHeap);
  PrintArray(testingArray, size);
  TestInsertSort(size, RandomizedT, totalComparisonsInsert, totalSwapsInsert);
  PrintArray(testingArray, size);
{
  GenerateArray(generatedArray, size, RandomizedT);
  PrintArray(generatedArray, size);
  TestHeapSort(size, RandomizedT, totalComparisonsHeap, totalSwapsHeap);
  PrintArray(generatedArray, size);
  TestInsertSort(size, RandomizedT, totalComparisonsInsert, totalSwapsInsert);
  PrintArray(generatedArray, size);

  if COMPRESSED_DEBUG_PRINT then
    WriteLn(
      '(', (totalComparisonsInsert / 4):0:2, ',', (totalSwapsInsert / 4):0:2, ',',
      (totalComparisonsHeap / 4):0:2, ',', (totalSwapsHeap / 4):0:2, ')'
    )
  else begin
    WriteLn('Insert average comparisons: ', (totalComparisonsInsert / 4):0:2);
    WriteLn('Insert average swaps: ', (totalSwapsInsert / 4):0:2);
    WriteLn('Heap average comparisons: ', (totalComparisonsHeap / 4):0:2);
    WriteLn('Heap average swaps: ', (totalSwapsHeap / 4):0:2);
  end;
  WriteLn;
}
end;

begin
  InitializeState;

{
  if STATIC_TEST then begin
    TestWithSize(10);
    TestWithSize(20);
    TestWithSize(50);
    TestWithSize(100);
  end else begin}
    ReadLn(n);
    if n > MAX_ARRAY_SIZE then
      WriteLn('Too big. Maximum accepted size: ', MAX_ARRAY_SIZE)
    else
      TestWithSize(n);
{  end;
}
  { TestWithSize(7); }
end.
