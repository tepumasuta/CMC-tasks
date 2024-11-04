program FUPASCALSum;

type
  Node = record val: Integer; next: ^Node end;
  List = ^Node;

var
  count: Integer;
  l: List;

procedure ReadList(var l: List; count: Integer);
var head: List; i: Integer;
begin
  for i := 1 to count do begin
    new(head);
    read(head^.val);
    head^.next := l;
    l := head;
  end
end;

procedure DisposeList(l: list);
var tail: List;
begin
  while l <> nil do begin
    tail := l^.next;
    dispose(l);
    l := tail;
  end
end;

function FindMinMax(l: List; var min, max: List): Boolean;
var minFirst: Boolean;
begin
  minFirst := true;
  min := l;
  max := l;
  while l <> nil do begin
    if l^.val < min^.val then begin min := l; minFirst := false; end;
    if l^.val > max^.val then begin max := l; minFirst := true; end;
    l := l^.next;
  end;

  FindMinMax := minFirst;
end;

function Sum(l: list): Integer;
var pStart, pEnd: List; s: Integer;
begin
  if not FindMinMax(l, pStart, pEnd) then begin
      l := pStart;
      pStart := pEnd;
      pEnd := l;
  end;
  s := pEnd^.val;
  while pStart <> pEnd do begin
    s := s + pStart^.val;
    pStart := pStart^.next;
  end;
  Sum := s;
end;

begin
  Read(count);
  ReadList(l, count);
  WriteLn(Sum(l));
  DisposeList(l);
end.
