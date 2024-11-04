program Reverse;

type
  AllowedIndicies = 1..100;

var
  size: AllowedIndicies;
  i: AllowedIndicies;
  numbers: array[AllowedIndicies] of Real;
  tmp: Real;

begin
  for i := 1 to 100 do numbers[i] := 0.0;
  Read(size);
  
  for i := 1 to size do Read(numbers[i]);

  for i := 1 to size div 2 do begin
    tmp := numbers[i];
    numbers[i] := numbers[size - i + 1];
    numbers[size - i + 1] := tmp;
  end;

  for i := 1 to size do Write(numbers[i], ' ');
  WriteLn;
end.
