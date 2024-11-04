program FUPASCALFrequent;

const MAXN = 10000;
type word = packed array[1..MAXN] of 'a'..'z';
var
  W: word;
  N, i: Integer;

function frequent(var W: word; N: Integer): Char;
var
  f: array['a'..'z'] of Integer;
  i: Integer;
  p, p_max: char;
begin
  for p := 'a' to 'z' do f[p] := 0;
  for i := 1 to N do f[W[i]] := f[W[i]] + 1;

  p_max := 'a';
  for p := 'a' to 'z' do
    if f[p] >= f[p_max] then p_max := p;

  frequent := p_max;
end;

{ function frequent(var W: word; N: Integer): Char;
var
  p, p_max: Char;
  i, p_max_count, p_count: Integer;
begin
  p_max := 'a';
  p_max_count := 0;
  for p := 'a' to 'z' do begin
    p_count := 0;
    for i := 1 to N do if W[i] = p then p_count := p_count + 1;
    if p_count > p_max_count then begin
      p_max := p;
      p_max_count := p_count;
    end;
  end;
  frequent := p_max;
end; }

begin
  ReadLn(N);
  for i := 1 to N do
    Read(W[i]);

  WriteLn(frequent(W, N));
end.
