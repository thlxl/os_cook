#ifndef _MQUEUE_H
#define _MQUEUE_H

#include "type.h"
#include "task.h"
#include "mem.h"

typedef struct messageQueue
{
	u8_t *pcHead;					/*队列头指针*/
	u8_t *pcTail;					/*队列尾指针*/
	u8_t *pcWriteTo;				/*下一个空闲区域指针*/

	u8_t *pcReadFrom;			/*指向最后一项出队时候的首地址*/

	tcb_t* xTasksWaitingToSend;		/*等待发送任务列表，那些因为队列满入队失败而进入阻塞状态的任务将挂载在此列表上*/
	tcb_t* xTasksWaitingToReceive;	/*等待接收任务列表，那些因为队列空出队失败而进入阻塞状态的任务将挂载在此列表上 */

	volatile u32_t uxMessagesWaiting;/*队列中消息数目 */
	u32_t uxLength;			/*队列长度 */
	u32_t uxItemSize;			/*队列每一条消息的大小，单位字节 */

	volatile int8_t cRxLock;		/*当队列上锁以后用来统计从队列中接收到的消息数量，也就是出队的消息数量，当队列没有上锁的话此字段为-1 */
	volatile int8_t cTxLock;		/*当队列上锁以后用来统计发送到队列中的消息数量，也就是入队的消息数量，当队列没有上锁的话此字段为-1 */


} messageQueue_t;

typedef messageQueue_t Queue_t;

/*写队列的方式*/
#define	SEND_TO_BACK		((u32_t)0)  /*< 写入队列尾部 */
#define	SEND_TO_FRONT		((u32_t)1)  /*< 写入队列头部 */
#define OVERWRITE			((u32_t)2)  /*< 覆盖写入 */

typedef void * mQueueHandle_t;

#endif/* _MQUEUE_H */