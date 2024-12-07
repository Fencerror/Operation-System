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
