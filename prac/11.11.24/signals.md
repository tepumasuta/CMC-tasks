# Сигналы

Сигналы &mdash; средство уведомления процесса о том, что какое-то событие произошло (синхронно или асинхронно)

```bash
kill -l # Выводит все сигналы
kill Np2 # SIGTERM (посылает процессу Np2)
kill -9 Np2 # SIGKILL
ps # Выводит текущие запущенные процессы
pkill name # Ищет процесс с именем name и ему послыает сигнал
```

```c
#include <signal.h>
```

```c
int kill(pid_t pid, int sig);
```

| pid  | значит                                 |
| ---- | -------------------------------------- |
| > 0  | номер pid                              |
|  0   | всей группе                            |
| -1   | всем &laquo;своим&raquo;, с тем же gid |
| < -1 | всем их группы `gid = abs(pid)`        |

```c
// Эквивалентны:
kill(getpid(), sig);
raise(sig);
```

```c
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);

/* Что показала она */
void (*signal(int sig, void (*handler)(int))(int); // Установить обработчик, возвращает предыдущий обработчик

void hand(int s) { /* ... */ }
signal(SIGINT, hand);
```

! Это &laquo;ненадёжные&raquo; сигналы\
&nbsp;

```c
int pause(void); // Ждёт любой сигнал
```

| pause | значит                 |
| ----- | ---------------------- |
| -1    | Не игнорируемый сигнал |

```c
unsigned sleep(unsigned seconds);
```

| sleep | значит            |
| ----- | ----------------- |
| 0     | Успешно           |
| > 0   | Сколько недоспали |

```c
int nanosleep(const struct timespec *duration, struct timespec *_Nullable rem);
// Что она показала:
{ time_t tv_sec;
  long   tv_nsec; } // 0..9..9
                    //    ^^^^ -- 9 девяток
```

```c
int alarm(unsigned seconds); // Кидает SIGALRM
```

| alarm | значит        |
| ----- | ------------- |
| 0     | Отключить     |
| > 0   | Через сколько |

```c
fork(); // В сыне SIGALRM сбрасывается
```

SIGCHLD &mdash; сигнал, посылаемый при изменении статуса дочернего процесса отцу (завершён, приостановлен или возобновлен)

SIGALRM &mdash; будильник, кидается через `alarm`

SIGKILL, SIGSTOP &mdash; не игнорируются и не обрабатываются

SIGTERM &mdash; сигнал завершения (грациозного завершения, например надо файлы там закрыть, буферы скинуть программе и т.д., она имеет возможность это сделать, перехватив сигнал)\
SIGTSTP &mdash; сигнал останова (^Z, ctrl+z &ndash; в консоли)\
SIGCONT &mdash; восстановить приостановленный процесс\
SIGTTIN &mdash; при попытке чтения с терминала, приостанавливает процесс\
SIGTTOU &mdash; при попытке записи в терминал, приостанавливает процесс\
SIGINT &mdash; signal interrupt, прерывать процесс (^C, ctrl+c &ndash; в консоли)\
SIGQUIT &mdash; выход (^/ или  ^Del &ndash; в консоли)\
SIGABRT &mdash; при `abort()`\
SIGSEGV &mdash; segmentation fault\
SIGPIPE &mdash; при попытке записи, когда закрыты все дескрипторы на чтение\
SIGUSR1, SIGUSR2 &mdash; выделены для пользователей\
SIGTRAP &mdash; при трассировке процессов (отладке)

В памяти процесса есть поля для способов обработки сигналов (IGNORE &ndash; SIG_IGN, DEFAULT &ndash; SIG_DFL, ...)

```bash
bg # Возобновляет процесс в фоне
fg # Возобновляет процесс или выводит из фонового режима
```
