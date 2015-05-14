#ifndef _EVENT_SERVER_H_
#define _EVENT_SERVER_H_


extern void EventServer_Init();
extern void EventServer_DeInit();
extern void *EventServer_Loop(void *pParam);

#endif /* _EVENT_SERVER_H_ */
