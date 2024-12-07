## Задание №1:
Требуется безопасно и эффективно реализовать монитор с двумя потоками: потоком-поставщиком и потоком-потребителем. 
Поток-поставщик должен с задержкой в одну секунду инициировать условное событие, о чём должен выводить сообщение на экран. 
Поток-потребитель должен это условное событие получать, о чём также должен сообщать на экране. 
В итоге на экране должна появляться серия чередующихся сообщений об отправлении события и его обработке. 
Сообщения не должны нарушать очерёдность. Ожидание события должно происходить с минимальным потреблением процессорного времени, 
т. е. быть практически равным нулю. Подразумевается, что условное событие может содержать несериализуемые данные (скажем, передаётся экземпляр класса по указателю).

## Решение:

Для реализации описанного монитора с потоками в Java можно использовать возможности стандартной библиотеки, такие как ReentrantLock и условные переменные (condition variables). Это позволит обеспечить синхронизацию и минимальное потребление ресурсов во время ожидания.

Вот пример кода:

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
