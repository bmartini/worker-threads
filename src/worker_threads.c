#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define TASK_NB 3
#define TYPE1 0
#define TYPE2 1

enum pt_state {
	SETUP,
	IDLE,
	JOB,
	DIE
};

typedef struct pt_info {
	int task;
	int size;
	enum pt_state state;
	pthread_cond_t work_cond;
	pthread_mutex_t work_mtx;
	pthread_cond_t boss_cond;
	pthread_mutex_t boss_mtx;
} pt_info;

pthread_t threads[TASK_NB][2];
pt_info shared_info[TASK_NB][2];

void *task_type1(void *arg)
{
	pt_info *info = (pt_info *) arg;
	int task = info->task;
	int size = 0;

	// cond_wait mutex must be locked before we can wait
	pthread_mutex_lock(&(info->work_mtx));

	printf("<worker %i:%i> start\n", task, TYPE1);

	// ensure boss is waiting
	pthread_mutex_lock(&(info->boss_mtx));

	// signal to boss that setup is complete
	info->state = IDLE;

	// wake-up signal
	pthread_cond_signal(&(info->boss_cond));
	pthread_mutex_unlock(&(info->boss_mtx));

	while (1) {
		pthread_cond_wait(&(info->work_cond), &(info->work_mtx));

		if (DIE == info->state) {
			// kill thread
			break;
		}

		if (IDLE == info->state) {
			// accidental wake-up
			continue;
		}
		// have a JOB to do
		size = info->size;

		// do blocking task
		printf("<worker %i:%i> JOB start\n", task, TYPE1);
		usleep(size*1000000);
		printf("<worker %i:%i> JOB end\n", task, TYPE1);

		// ensure boss is waiting
		pthread_mutex_lock(&(info->boss_mtx));

		// indicate that job is done
		info->state = IDLE;

		// wake-up signal
		pthread_cond_signal(&(info->boss_cond));
		pthread_mutex_unlock(&(info->boss_mtx));
	}

	pthread_mutex_unlock(&(info->work_mtx));
	pthread_exit(NULL);
}

void *task_type2(void *arg)
{
	pt_info *info = (pt_info *) arg;
	int task = info->task;
	int size = 0;

	// cond_wait mutex must be locked before we can wait
	pthread_mutex_lock(&(info->work_mtx));

	printf("<worker %i:%i> start\n", task, TYPE2);

	// ensure boss is waiting
	pthread_mutex_lock(&(info->boss_mtx));

	// signal to boss that setup is complete
	info->state = IDLE;

	// wake-up signal
	pthread_cond_signal(&(info->boss_cond));
	pthread_mutex_unlock(&(info->boss_mtx));

	while (1) {
		pthread_cond_wait(&(info->work_cond), &(info->work_mtx));

		if (DIE == info->state) {
			// kill thread
			break;
		}

		if (IDLE == info->state) {
			// accidental wake-up
			continue;
		}
		// have a JOB to do
		size = info->size;

		// do blocking task
		printf("<worker %i:%i> JOB start\n", task, TYPE2);
		usleep(size*1000000);
		printf("<worker %i:%i> JOB end\n", task, TYPE2);

		// ensure boss is waiting
		pthread_mutex_lock(&(info->boss_mtx));

		// indicate that job is done
		info->state = IDLE;

		// wake-up signal
		pthread_cond_signal(&(info->boss_cond));
		pthread_mutex_unlock(&(info->boss_mtx));
	}

	pthread_mutex_unlock(&(info->work_mtx));
	pthread_exit(NULL);
}

void task_start(int task, int type, int size)
{
	pt_info *info = &(shared_info[task][type]);

	// ensure worker is waiting
	pthread_mutex_lock(&(info->work_mtx));

	// set job information & state
	info->size = size;
	info->state = JOB;

	// wake-up signal
	pthread_cond_signal(&(info->work_cond));
	pthread_mutex_unlock(&(info->work_mtx));
}

void task_wait(int task, int type)
{
	pt_info *info = &(shared_info[task][type]);

	while (1) {
		pthread_cond_wait(&(info->boss_cond), &(info->boss_mtx));

		if (IDLE == info->state) {
			break;
		}
	}
}

void app_init()
{
	pt_info *info = NULL;

	for (int i = 0; i < TASK_NB; i++) {
		info = &(shared_info[i][TYPE1]);

		info->task = i;
		info->size = 0;
		info->state = SETUP;

		pthread_cond_init(&(info->work_cond), NULL);
		pthread_mutex_init(&(info->work_mtx), NULL);
		pthread_cond_init(&(info->boss_cond), NULL);
		pthread_mutex_init(&(info->boss_mtx), NULL);

		pthread_mutex_lock(&(info->boss_mtx));

		pthread_create(&threads[i][TYPE1], NULL, task_type1,
			       (void *)info);

		task_wait(i, TYPE1);
	}

	for (int i = 0; i < TASK_NB; i++) {
		info = &(shared_info[i][TYPE2]);

		info->task = i;
		info->size = 0;
		info->state = SETUP;

		pthread_cond_init(&(info->work_cond), NULL);
		pthread_mutex_init(&(info->work_mtx), NULL);
		pthread_cond_init(&(info->boss_cond), NULL);
		pthread_mutex_init(&(info->boss_mtx), NULL);

		pthread_mutex_lock(&(info->boss_mtx));

		pthread_create(&threads[i][TYPE2], NULL, task_type2,
			       (void *)info);

		task_wait(i, TYPE2);
	}
}

void app_exit()
{
	pt_info *info = NULL;

	for (int i = 0; i < TASK_NB; i++) {
		for (int d = 0; d < 2; d++) {
			info = &(shared_info[i][d]);

			// ensure the worker is waiting
			pthread_mutex_lock(&(info->work_mtx));

			printf("app_exit: send DIE to <worker %i:%i>\n", i, d);
			info->state = DIE;

			// wake-up signal
			pthread_cond_signal(&(info->work_cond));
			pthread_mutex_unlock(&(info->work_mtx));

			// wait for thread to exit
			pthread_join(threads[i][d], NULL);

			pthread_mutex_destroy(&(info->work_mtx));
			pthread_cond_destroy(&(info->work_cond));

			pthread_mutex_unlock(&(info->boss_mtx));
			pthread_mutex_destroy(&(info->boss_mtx));
			pthread_cond_destroy(&(info->boss_cond));

		}
	}
}

int main(int argc, char *argv[])
{
	app_init();

	printf("Init done\n");

	task_start(0, TYPE1, 1);
	task_start(1, TYPE1, 1);
	task_wait(1, TYPE1);
	task_wait(0, TYPE1);

	task_start(0, TYPE2, 5);
	task_start(0, TYPE1, 2);
	task_wait(0, TYPE2);
	task_wait(0, TYPE1);

	app_exit();

	return 0;
}
