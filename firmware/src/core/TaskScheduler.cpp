#include "TaskScheduler.h"

/*
 * TaskScheduler'ı ilk duruma getirir.
 * Tüm görevler temizlenir ve görev sayısı sıfırlanır.
 */
void TaskScheduler::begin()
{
    taskCount = 0;

    // Tüm görev slotlarını boş hale getir.
    for (uint8_t i = 0; i < Constants::MAX_TASK_COUNT; ++i)
    {
        tasks[i] = Task{};
    }
}

/*
 * Scheduler'ın sürekli çağrılan fonksiyonudur.
 * Süresi dolan görevlerin callback fonksiyonlarını çalıştırır.
 */
void TaskScheduler::update()
{
    const uint32_t currentTimeMs = millis();

    // Tüm görevleri sırayla kontrol et.
    for (uint8_t i = 0; i < Constants::MAX_TASK_COUNT; ++i)
    {
        Task& task = tasks[i];

        /*
         * Aşağıdaki durumlarda bu görevi atla:
         * - Slot boşsa
         * - Görev devre dışıysa
         * - Callback fonksiyonu tanımlı değilse
         */
        if (!task.occupied ||
            !task.enabled ||
            task.callback == nullptr)
        {
            continue;
        }

        /*
         * Belirlenen süre dolmuşsa görevi çalıştır.
         */
        if (currentTimeMs - task.lastRunMs >= task.intervalMs)
        {
            // Son çalışma zamanını güncelle.
            task.lastRunMs = currentTimeMs;

            // Kullanıcının verdiği fonksiyonu çalıştır.
            task.callback();
        }
    }
}

/*
 * Scheduler'a yeni bir görev ekler.
 *
 * callback        : Çalıştırılacak fonksiyon
 * intervalMs      : Kaç ms'de bir çalışacağı
 * runImmediately  : İlk update() çağrısında hemen çalışsın mı?
 *
 * Başarılı olursa görev ID'si döner.
 * Yer yoksa INVALID_TASK_ID döner.
 */
int8_t TaskScheduler::addTask(
    TaskCallback callback,
    uint32_t intervalMs,
    bool runImmediately
)
{
    // Geçersiz parametre kontrolü.
    if (callback == nullptr || intervalMs == 0)
    {
        return INVALID_TASK_ID;
    }

    // İlk boş görev slotunu bul.
    for (uint8_t i = 0; i < Constants::MAX_TASK_COUNT; ++i)
    {
        if (tasks[i].occupied)
        {
            continue;
        }

        // Görev bilgilerini kaydet.
        tasks[i].callback = callback;
        tasks[i].intervalMs = intervalMs;
        tasks[i].enabled = true;
        tasks[i].occupied = true;

        const uint32_t currentTimeMs = millis();

        /*
         * Eğer hemen çalışması isteniyorsa,
         * son çalışma zamanı geçmişe alınır.
         * Böylece ilk update() çağrısında çalışır.
         */
        tasks[i].lastRunMs = runImmediately
            ? currentTimeMs - intervalMs
            : currentTimeMs;

        ++taskCount;

        return static_cast<int8_t>(i);
    }

    // Boş görev slotu bulunamadı.
    return INVALID_TASK_ID;
}

/*
 * Bir görevi aktif veya pasif hale getirir.
 */
bool TaskScheduler::setTaskEnabled(
    uint8_t taskId,
    bool enabled
)
{
    if (!isValidTaskId(taskId))
    {
        return false;
    }

    tasks[taskId].enabled = enabled;

    /*
     * Görev tekrar aktif edildiğinde
     * zamanlayıcı sıfırlanır.
     */
    if (enabled)
    {
        tasks[taskId].lastRunMs = millis();
    }

    return true;
}

/*
 * Bir görevin zamanlayıcısını sıfırlar.
 * Böylece belirtilen süre yeniden sayılmaya başlanır.
 */
bool TaskScheduler::resetTask(uint8_t taskId)
{
    if (!isValidTaskId(taskId))
    {
        return false;
    }

    tasks[taskId].lastRunMs = millis();

    return true;
}

/*
 * Bir görevi scheduler'dan tamamen siler.
 */
bool TaskScheduler::removeTask(uint8_t taskId)
{
    if (!isValidTaskId(taskId))
    {
        return false;
    }

    // Görev slotunu tamamen temizle.
    tasks[taskId] = Task{};

    if (taskCount > 0)
    {
        --taskCount;
    }

    return true;
}

/*
 * Scheduler'da kayıtlı görev sayısını döndürür.
 */
uint8_t TaskScheduler::getTaskCount() const
{
    return taskCount;
}

/*
 * Verilen görev ID'sinin geçerli olup olmadığını kontrol eder.
 *
 * Geçerli olması için:
 * - Dizi sınırları içinde olmalı.
 * - O görev slotu gerçekten kullanılıyor olmalı.
 */
bool TaskScheduler::isValidTaskId(uint8_t taskId) const
{
    return taskId < Constants::MAX_TASK_COUNT &&
           tasks[taskId].occupied;
}