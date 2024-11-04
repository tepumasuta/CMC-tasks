program Order;

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

{ Да, я знаю, можно сортировки O(n*logn), и всё же, мне кажется список как структура не очень
  подходит для имплементации алгоритмов сортировки }
procedure Sort(l: List);
var p: List; tmp: Integer;
begin
  while l <> nil do begin
    p := l^.next;
    while p <> nil do begin
      if l^.val > p^.val then begin
        tmp := l^.val;
        l^.val := p^.val;
        p^.val := tmp;
      end;
      p := p^.next;
    end;
    l := l^.next;
  end;
end;

begin
  Read(count);
  ReadList(l, count);
  Sort(l);
  PrintList(l);
  WriteLn;
  DisposeList(l);
end.
