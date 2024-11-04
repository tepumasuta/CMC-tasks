program FUPASCALGroups;

type
  Node = record val: Integer; next: ^Node end;
  List = ^Node;

var
  count: Integer;
  l: List;

procedure PrintList(l: list);
begin
  while l <> nil do begin
    Write(l^.val, ' ');
    l := l^.next;
  end
end;

procedure ReadList(var l: List; count: Integer);
var tail: List; i: Integer;
begin
  if count > 0 then begin
    new(l);
    read(l^.val);
    l^.next := nil;
    tail := l;
    for i := 2 to count do begin
      new(tail^.next);
      read(tail^.next^.val);
      tail^.next^.next := nil;
      tail := tail^.next;
    end
  end;
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

procedure Groups(l: list);
var p, tmp: List;
begin
  while l <> nil do begin
    p := l^.next;
    while (p <> nil) and (p^.val = l^.val) do begin
      tmp := p^.next;
      dispose(p);
      p := tmp;
    end;
    l^.next := p;
    l := l^.next;
  end;
end;

begin
  Read(count);
  ReadList(l, count);
  Groups(l);
  PrintList(l);
  WriteLn;
  DisposeList(l);
end.
