program Max3;

var
  m1, m2, m3: Integer;
  number: Integer;

begin
  m1 := 0;
  m2 := 0;
  m3 := 0;

  read(number);
  while number <> 0 do begin
    if number > m1 then begin
      m3 := m2;
      m2 := m1;
      m1 := number;
    end else if number > m2 then begin
      m3 := m2;
      m2 := number;
    end else if number > m3 then
      m3 := number;

    read(number);
  end;
  WriteLn(m1, ' ', m2, ' ', m3);
end.
