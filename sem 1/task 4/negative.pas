program Negative;

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

procedure Delete(var l: List);
var p, tmp: List;
begin
  while (l <> nil) and (l^.val < 0) do begin
    tmp := l;
    l := l^.next;
    dispose(tmp);
  end;

  p := l;

  while p <> nil do begin
    while (p^.next <> nil) and (p^.next^.val < 0) do begin
      tmp := p^.next;
      p^.next := p^.next^.next;
      dispose(tmp);
    end;

    p := p^.next;
  end;
end;

begin
  Read(count);
  ReadList(l, count);
  Delete(l);
  PrintList(l);
  WriteLn;
  DisposeList(l);
end.
