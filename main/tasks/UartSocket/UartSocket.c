#include "UartSocket.h"

static void task(void *para);

struct UartSocket_Module UartSocket = {
	.task = task,
	.taskName = "UartSocket",
}

static void task(void *para) {

}
