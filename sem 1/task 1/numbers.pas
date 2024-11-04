program Numbers;

var
  d1, d2, d3: Integer;

begin
  for d1 := 1 to 9 do begin
    for d2 := 0 to 9 do begin
      if d1 <> d2 then
      for d3 := 0 to 9 do begin
        if (d1 <> d3) and (d2 <> d3) then
          WriteLn(d1,d2,d3);
      end;
    end;
  end;
end.
