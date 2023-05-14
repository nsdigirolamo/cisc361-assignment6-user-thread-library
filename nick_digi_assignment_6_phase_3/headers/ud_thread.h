/* 
 * thread library function prototypes
 */
void t_create(void(*function)(int), int thread_id, int priority);
void t_yield(void);
void t_init(void);
void t_terminate(void);
void t_shutdown(void);

typedef void sem_t;

int sem_init(sem_t **sp, unsigned int count);
void sem_wait(sem_t *sp);
void sem_signal(sem_t *sp);
void sem_destroy(sem_t **sp);
