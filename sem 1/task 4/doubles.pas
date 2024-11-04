program FUPASCALDoubles;

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

function Doubles(l: list): Boolean;
var p: List; found: Boolean;
begin
  found := false;
  while (l <> nil) and (not found) do begin
    p := l^.next;
    while p <> nil do begin
      if p^.val = l^.val then found := true;
      p := p^.next;
    end;
    l := l^.next;
  end;
  Doubles := found;
end;

begin
  Read(count);
  ReadList(l, count);
  if Doubles(l) then WriteLn('YES') else WriteLn('NO');
  DisposeList(l);
end.
