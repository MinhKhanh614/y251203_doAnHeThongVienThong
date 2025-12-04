#include "MK_Message.h"

// Queue handles (khởi tạo trong app_run)
QueueHandle_t keypadQueue = NULL;
QueueHandle_t mainQueue = NULL;
QueueHandle_t displayQueue = NULL;

void sendMessage(QueueHandle_t queue, const Message &msg)
{
    if (queue != NULL)
    {
        xQueueSend(queue, &msg, portMAX_DELAY);
    }
}

bool receiveMessage(QueueHandle_t queue, Message &msg, int timeoutMs)
{
    if (queue == NULL)
        return false;

    TickType_t timeout = (timeoutMs == 0) ? 0 : pdMS_TO_TICKS(timeoutMs);
    return xQueueReceive(queue, &msg, timeout) == pdTRUE;
}
