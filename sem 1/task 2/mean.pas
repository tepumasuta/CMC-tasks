program Mean;

type
  Indicies = 1..100;

var
  size, i: Indicies;
  closest_i: Integer;
  numbers: array[Indicies] of Real;
  avg, diff: Real;

begin
  Read(size);
  for i := size + 1 to 100 do numbers[i] := 0;

  for i := 1 to size do begin
    Read(numbers[i]);
    avg := avg + numbers[i];
  end;
  avg := avg / size;

  
  closest_i := 1;
  diff := abs(numbers[1] - avg);

  for i := 2 to size do begin
    if abs(avg - numbers[i]) < diff then begin
      diff := abs(avg - numbers[i]);
      closest_i := i;
    end;
  end;

  WriteLn(closest_i);
end.
