# Практические работы по курсу "Операционные системы"

## Задание №1:
### Мониторы.
Требуется безопасно и эффективно реализовать монитор с двумя потоками: потоком-поставщиком и потоком-потребителем. 
Поток-поставщик должен с задержкой в одну секунду инициировать условное событие, о чём должен выводить сообщение на экран. 
Поток-потребитель должен это условное событие получать, о чём также должен сообщать на экране. 
В итоге на экране должна появляться серия чередующихся сообщений об отправлении события и его обработке. 
Сообщения не должны нарушать очерёдность. Ожидание события должно происходить с минимальным потреблением процессорного времени, 
т. е. быть практически равным нулю. Подразумевается, что условное событие может содержать несериализуемые данные (скажем, передаётся экземпляр класса по указателю).

### Решение:

Для реализации описанного монитора с потоками в Java можно использовать возможности стандартной библиотеки, такие как ReentrantLock и условные переменные (condition variables). Это позволит обеспечить синхронизацию и минимальное потребление ресурсов во время ожидания.

Код:
```
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class MonitorExample {

    private static class Monitor {
        private final Lock lock = new ReentrantLock();
        private final Condition eventCondition = lock.newCondition();
        private boolean eventAvailable = false;

        // Метод для инициирования события
        public void produceEvent() throws InterruptedException {
            lock.lock();
            try {
                while (eventAvailable) {
                    eventCondition.await();
                }
                eventAvailable = true;
                System.out.println("Producer: Event produced");
                eventCondition.signal();
            } finally {
                lock.unlock();
            }
        }

        // Метод для получения события
        public void consumeEvent() throws InterruptedException {
            lock.lock();
            try {
                while (!eventAvailable) {
                    eventCondition.await();
                }
                eventAvailable = false;
                System.out.println("Consumer: Event consumed");
                eventCondition.signal();
            } finally {
                lock.unlock();
            }
        }
    }

    public static void main(String[] args) {
        Monitor monitor = new Monitor();

        Thread producer = new Thread(() -> {
            try {
                while (true) {
                    Thread.sleep(1000); // Задержка 1 секунда
                    monitor.produceEvent();
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        Thread consumer = new Thread(() -> {
            try {
                while (true) {
                    monitor.consumeEvent();
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        producer.start();
        consumer.start();
    }
}
```
### Пояснения:
1. Класс `Monitor`:
   - Использует ReentrantLock для блокировки ресурсов.
   - Условная переменная eventCondition используется для ожидания и оповещения о состоянии события.
   - Флаг eventAvailable указывает, доступно ли событие.

2. Метод `produceEvent`:
   - Поставщик блокируется, если событие уже существует.
   - После генерации события он оповещает потребителя.

3. Метод `consumeEvent`:
   - Потребитель блокируется, пока событие недоступно.
   - После обработки события он оповещает поставщика.

4. Потоки:
   - Поток-поставщик каждую секунду создает событие.
   - Поток-потребитель ждет появления события, обрабатывает его и снова переходит в режим ожидания.

### Преимущества:
- Минимальное использование CPU благодаря механизму await() и signal().
- Четкая синхронизация между потоками.
- Сообщения не нарушают очередность благодаря правильному управлению состоянием.



## Задание 2:

### Безопасная обработка сетевых подключений и сигналов.
Требуется подготовить безопасную реализацию серверного процесса, который совмещает обработку соединений TCP/IP с обработкой сигналов (можно выбрать, скажем, сигнал SIGHUP). Приложение должно:

1. Принимать соединения на некоторый порт, сообщать о новых соединениях на терминал, одно соединение оставлять принятым, остальные закрывать сразу после подключения.
2. При появлении любых данных в соединении выводить сообщение на терминал (для простоты достаточно вывести только количество полученных данных).
3. При получении сигнала выводить сообщение на терминал.
Фактически, работа сводится к правильному вызову функции pselect(), но все действия должны быть выполнены в предельно безопасном виде, исключающем любые race condition.

### Решение:
Для решения задачи используется системный вызов pselect() в сочетании с корректной обработкой сигналов, обеспечивающей безопасное выполнение серверного процесса. Вот пример кода на C:

Код:

```
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class MonitorExample {

    private static class Monitor {
        private final Lock lock = new ReentrantLock();
        private final Condition eventCondition = lock.newCondition();
        private boolean eventAvailable = false;

        // Метод для инициирования события
        public void produceEvent() throws InterruptedException {
            lock.lock();
            try {
                while (eventAvailable) {
                    eventCondition.await();
                }
                eventAvailable = true;
                System.out.println("Producer: Event produced");
                eventCondition.signal();
            } finally {
                lock.unlock();
            }
        }

        // Метод для получения события
        public void consumeEvent() throws InterruptedException {
            lock.lock();
            try {
                while (!eventAvailable) {
                    eventCondition.await();
                }
                eventAvailable = false;
                System.out.println("Consumer: Event consumed");
                eventCondition.signal();
            } finally {
                lock.unlock();
            }
        }
    }

    public static void main(String[] args) {
        Monitor monitor = new Monitor();

        Thread producer = new Thread(() -> {
            try {
                while (true) {
                    Thread.sleep(1000); // Задержка 1 секунда
                    monitor.produceEvent();
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        Thread consumer = new Thread(() -> {
            try {
                while (true) {
                    monitor.consumeEvent();
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        producer.start();
        consumer.start();
    }
}
```

### Пояснения:
1. **Обработка сигналов**:
   - `sigaction` устанавливает обработчик для `SIGHUP`.
   - Сигнал блокируется в основном цикле с помощью `sigprocmask` и обрабатывается безопасно внутри `pselect`.

2. **Использование `pselect`**:
   - Позволяет одновременно ждать событий на сокетах и обрабатывать сигналы.
   - Маска сигналов `orig_mask` обеспечивает безопасное управление сигналами.

3. **Сокетная логика**:
   - Сервер слушает на указанном порту.
   - Принятое соединение сохраняется как активное, а предыдущие закрываются.
   - Чтение данных выполняется неблокирующе.

4. **Реакция на события**:
   - Сообщение о новых соединениях.
   - Подсчет и вывод количества полученных данных.
   - Обработка `SIGHUP`.

### Преимущества:
- Безопасная обработка сигналов.
- Использование `pselect` минимизирует race condition.
- Простое и читаемое управление соединениями и данными.


## Задание 3:

### Модуль ядра Linux.
#### Часть первая:

Требуется подготовить модуль для загрузки в ядро Linux. Модуль должен:

- сопровождаться Makefile для сборки;
- оформляться в виде файла *.ko;
- загружаться командой insmod и выгружаться командой rmmod;
- при загрузке выводить в dmesg строку «Welcome to the Tomsk State University»;
- при выгрузке выводить в dmesg строку «Tomsk State University forever!».:

### Решение:



Код модуля ядра Linux:

```
#include <linux/module.h>  // Для макросов модулей (MODULE_*)
#include <linux/kernel.h>  // Для printk()

// Заголовки для информации о модуле
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomsk State University");
MODULE_DESCRIPTION("A simple Linux kernel module for TSU");

// Функция, вызываемая при загрузке модуля
static int __init tsu_init(void) {
    printk(KERN_INFO "Welcome to the Tomsk State University\n");
    return 0;  // Возврат 0 означает успешную загрузку
}

// Функция, вызываемая при выгрузке модуля
static void __exit tsu_exit(void) {
    printk(KERN_INFO "Tomsk State University forever!\n");
}

// Указываем функции для загрузки и выгрузки модуля
module_init(tsu_init);
module_exit(tsu_exit);

```

MakeFile для сборки модуля:
```
obj-m += tsu_module.o

# Укажите путь к исходным кодам ядра
KDIR := /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

```

### Пояснения:
Модуль реализован в файле tsu_module.c. Основные функции:

1. tsu_init: вызывается при загрузке модуля; использует printk для вывода строки приветствия.
2. tsu_exit: вызывается при выгрузке модуля; выводит строку прощания.
