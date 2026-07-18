/*
 * ------------------------------------------------------------------
 * TaskScheduler
 *
 * Delay() kullanılmadan zaman tabanlı görev yönetimi sağlar.
 * Her görev belirlenen zaman aralığında otomatik olarak çalıştırılır.
 *
 * Kullanım:
 *      scheduler.begin();
 *      scheduler.addTask(...);
 *
 * Loop içerisinde:
 *      scheduler.update();
 * ------------------------------------------------------------------
 */

#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <Arduino.h>
#include "Constants.h"

/*
 * TaskScheduler
 *
 * Robot içerisindeki periyodik görevleri delay() kullanmadan
 * belirli zaman aralıklarında çalıştırmak için kullanılır.
 *
 * Örnek:
 *  - OLED güncelleme
 *  - Batarya kontrolü
 *  - RFID okuma
 *  - Sensör kontrolü
 */
class TaskScheduler
{
public:

    /*
     * Görev olarak eklenecek fonksiyonların
     * sahip olması gereken imza.
     *
     * Örnek:
     * void updateDisplay();
     */
    using TaskCallback = void (*)();

    /*
     * Geçersiz görev ID'sini temsil eder.
     * Yeni görev eklenemezse bu değer döndürülür.
     */
    static constexpr int8_t INVALID_TASK_ID = -1;

    /*
     * Scheduler'ı başlatır.
     * Tüm görevler temizlenir.
     */
    void begin();

    /*
     * Sürekli loop() içerisinde çağrılır.
     * Süresi dolan görevleri çalıştırır.
     */
    void update();

    /*
     * Yeni bir görev ekler.
     *
     * callback:
     *      Çalıştırılacak fonksiyon.
     *
     * intervalMs:
     *      Kaç milisaniyede bir çalışacağı.
     *
     * runImmediately:
     *      true ise ilk update() çağrısında
     *      hemen çalıştırılır.
     *
     * Dönüş:
     *      Başarılı olursa görev ID'si,
     *      başarısız olursa INVALID_TASK_ID.
     */
    int8_t addTask(
        TaskCallback callback,
        uint32_t intervalMs,
        bool runImmediately = false
    );

    /*
     * Belirtilen görevi aktif veya pasif yapar.
     */
    bool setTaskEnabled(
        uint8_t taskId,
        bool enabled
    );

    /*
     * Görevin zamanlayıcısını sıfırlar.
     * Süre yeniden sayılmaya başlanır.
     */
    bool resetTask(uint8_t taskId);

    /*
     * Görevi tamamen siler.
     */
    bool removeTask(uint8_t taskId);

    /*
     * Sistemde kayıtlı görev sayısını döndürür.
     */
    uint8_t getTaskCount() const;

private:

    /*
     * Scheduler içerisinde tutulan
     * tek bir görevin bilgileri.
     */
    struct Task
    {
        /*
         * Çalıştırılacak fonksiyon.
         */
        TaskCallback callback = nullptr;

        /*
         * Çalıştırma periyodu (ms).
         */
        uint32_t intervalMs = 0;

        /*
         * Son çalıştırıldığı zaman.
         */
        uint32_t lastRunMs = 0;

        /*
         * Görev aktif mi?
         */
        bool enabled = false;

        /*
         * Bu görev slotu kullanılıyor mu?
         */
        bool occupied = false;
    };

    /*
     * Scheduler'ın görev listesi.
     *
     * Maksimum görev sayısı
     * Constants::MAX_TASK_COUNT ile belirlenir.
     */
    Task tasks[Constants::MAX_TASK_COUNT];

    /*
     * Sistemde kayıtlı aktif görev sayısı.
     */
    uint8_t taskCount = 0;

    /*
     * Verilen görev ID'sinin geçerli olup
     * olmadığını kontrol eder.
     */
    bool isValidTaskId(uint8_t taskId) const;
};

#endif