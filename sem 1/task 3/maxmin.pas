program Maxmin;

type
  Vector = array[1..100] of Real;

const
  epsilon = 0.00001;

var
  a_iter: Real;
  b, c: Vector;
  t, it_res, it_res_sec, b_max, a_max, a_min: real;
  size: 1..100;
  i: 1..100;

{ Подумал, что можно не сохранять `a` в памяти и итеративно посчитать min и max }

function eq(x, y: real): boolean;
begin
  eq := abs(x - y) <= epsilon;
end;

begin
  Read(size);
  Read(a_iter);
  a_max := a_iter;
  a_min := a_iter;

  for i := 2 to size do begin
    Read(a_iter);
    if a_iter > a_max then a_max := a_iter;
    if a_iter < a_min then a_min := a_iter;
  end;
  Read(b[1]);
  b_max := b[1];
  for i := 2 to size do begin
    Read(b[i]);
    if b[i] > b_max then b_max := b[i];
  end;
  for i := 1 to size do Read(c[i]);

  t := 0;
  if (a_min > b_max) or eq(a_min, b_max) then begin
    it_res := b[1] + c[1];
    for i := 2 to size do begin
      if b[i] + c[i] > it_res then it_res := b[i] + c[i];
    end;

    t := it_res;

    it_res := c[1];
    for i := 2 to size do
      if c[i] < it_res then it_res := c[i];

    t := t + it_res;
  end else begin
    it_res := b[1];
    for i := 2 to size do
      if b[i] < it_res then it_res := b[i];

    t := it_res / a_max;

    it_res := c[1];
    for i := 2 to size do
      if c[i] > it_res then it_res := c[i];

    it_res_sec := b[1] + c[1];
    for i := 2 to size do begin
      if b[i] + c[i] < it_res_sec then it_res_sec := b[i] + c[i];
    end;

    t := t + it_res / it_res_sec;
  end;

  WriteLn(t:1:6);
end.
