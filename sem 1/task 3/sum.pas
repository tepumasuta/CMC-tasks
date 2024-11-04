program FUPASCALSum;

const MAXN = 1000;
type matrix = array[1..MAXN, 1..MAXN] of real;
var
  A: matrix;
  N, M, i, j: 1..MAXN;

function row_max(var A: matrix; M, i: Integer): Real;
var
  j: Integer;
  mx: Real;
begin
  mx := A[i, 1];
  for j := 2 to M do if A[i, j] > mx then mx := A[i, j];
  row_max := mx;
end;

function sum(var A: matrix; N, M: Integer): real;
var
  t: Real;
  i: Integer;
begin
  { Заметим, что x1*xn = xn*x1 -> в произведении до xk*xk все остальные можно возвести в квадрат }
  t := 0.0;
  for i := 1 to N div 2 do t := t + 2 * row_max(A, M, i) * row_max(A, M, N - i + 1);
  if (N mod 2) = 1 then t := t + sqr(row_max(A, M, N div 2 + 1));
  sum := t;
end;

begin
  Read(N, M);
  for i := 1 to N do
    for j := 1 to M do
      Read(A[i, j]);
  
  WriteLn(sum(A, N, M):1:6);
end.
