program Doubles;

var
  was: array[Char] of Boolean;
  text: array[1..1000] of Char;
  size, i, j: Integer;
  c: Char;

begin
  for i := 0 to 255 do was[chr(i)] := false;

  ReadLn(size);
  j := 1;
  for i := 1 to size do begin
    Read(c);
    if not was[c] then begin
      text[j] := c;
      was[c] := true;
      j := j + 1;
    end;
  end;

  for i := 1 to j-1 do Write(text[i]);
end.