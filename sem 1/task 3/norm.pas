program FUPASCALNorm;

const
  MAX_SIZE = 500;

type
  Matrix = array[1..MAX_SIZE] of array[1..MAX_SIZE] of Real;

var
  A, B, C, S: Matrix;
  i, j, N, M: Integer;
  t: Real;

procedure ReadMatrix(var A: Matrix; N, M: Integer);
var i, j: Integer;
begin
  for i := 1 to N do
    for j := 1 to M do
      Read(A[i, j]);
end;

procedure PrintMatrix(var A: Matrix; N, M: Integer);
var i, j: Integer;
begin
  for i := 1 to N do begin
    for j := 1 to M do
      Write(A[i, j]:1:6, ' ');
    WriteLn;
  end;
end;

function MaxAbsRow(var A: Matrix; cols, i: Integer): Real;
var
  t: Real;
  j: Integer;
begin
  t := abs(A[i, 1]);
  for j := 1 to cols do if abs(A[i, j]) > t then t := abs(A[i, j]);
  MaxAbsRow := t;
end;

function Norm(var A: Matrix; rows, cols: Integer): Real;
var
  t: Real;
  i: Integer;
begin
  t := 0;
  for i := 1 to rows do t := t + MaxAbsRow(A, cols, i);
  Norm := t;
end;

begin
  Read(N, M);
  ReadMatrix(A, N, M);
  ReadMatrix(B, N, M);
  ReadMatrix(C, N, M);
  for i := 1 to N do
    for j := 1 to M do
      S[i, j] := A[i, j] + B[i, j] + C[i, j];
  t := (Norm(A, N, M) + Norm(B, N, M) + Norm(C, N, M)) / Norm(S, N, M);
  WriteLn(t:1:5);
end.
