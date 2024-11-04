program FUPASCALAfter;

type
  Node = record val: Integer; next: ^Node end;
  List = ^Node;

var
  count, looking, insert: Integer;
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

function After(l: List; looking, insert: Integer): Boolean;
var p: List;
begin
  while l <> nil do begin
    if l^.val = looking then begin
      new(p);
      p^.next := l^.next;
      p^.val := insert;
      l^.next := p;
      l := p^.next;
    end else
      l := l^.next;
  end;
end;

begin
  Read(count);
  ReadList(l, count);
  Read(looking, insert);
  After(l, looking, insert);
  PrintList(l);
  WriteLn;
  DisposeList(l);
end.
