program Find;

var
  count, item, number, i, result: Integer;

begin
  read(count, item);
  result := 0;
  for i := 1 to count do
    if result = 0 then begin
      read(number);
      if number = item then
        result := i;
    end;

  WriteLn(result);
end.
